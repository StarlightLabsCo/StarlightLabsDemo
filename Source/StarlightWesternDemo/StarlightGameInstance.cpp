// Fill out your copyright notice in the Description page of Project Settings.


#include "StarlightGameInstance.h"
#include "StarlightNPC.h"
#include "WebSocketsModule.h"
#include "Json.h"
#include "Misc/Base64.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "UObject/ConstructorHelpers.h"
#include <Kismet/GameplayStatics.h>

void UStarlightGameInstance::Init()
{
	Super::Init();

	// -- Hide Quest Widget ---
	HideQuestWidget();

	// --- Audio Capture ---
	if (!FModuleManager::Get().IsModuleLoaded("AudioCapture"))
	{
		FModuleManager::Get().LoadModule("AudioCapture");
	}

	// --- WebSockets ---
	ConnectToWebSocketServer(0);
}

void UStarlightGameInstance::ConnectToWebSocketServer(int32 RetryCount = 0)
{
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	WebSocket = FWebSocketsModule::Get().CreateWebSocket("ws://localhost:8080");

	// Connection Status
	WebSocket->OnConnected().AddLambda([]()
		{
			UE_LOG(LogTemp, Warning, TEXT("Connected to WebSocket Server!"));
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Connected to ws://localhost:8080 websocket server");
		});

	WebSocket->OnConnectionError().AddLambda([RetryCount, this](const FString& Error)
		{
			UE_LOG(LogTemp, Warning, TEXT("Error connecting to WebSocket Server: %s"), *Error);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Error);

			if (RetryCount < RetryLimit)
			{
				FTimerHandle RetryTimerHandle;
				FTimerDelegate RetryDelegate = FTimerDelegate::CreateLambda([RetryCount, this]()
					{
						ConnectToWebSocketServer(RetryCount + 1);
					});

				GetWorld()->GetTimerManager().SetTimer(RetryTimerHandle, RetryDelegate, RetryDelay, false);
			}
		});

	WebSocket->OnClosed().AddLambda([RetryCount, this](int32 StatusCode, const FString& Reason, bool bWasClean)
		{
			UE_LOG(LogTemp, Warning, TEXT("WebSocket Server closed connection: %d %s %d"), StatusCode, *Reason, bWasClean);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "WebSocket Server closed connection");

			if (RetryCount < RetryLimit)
			{
				FTimerHandle RetryTimerHandle;
				FTimerDelegate RetryDelegate = FTimerDelegate::CreateLambda([RetryCount, this]()
					{
						ConnectToWebSocketServer(RetryCount + 1);
					});

				UWorld* World = GetWorld();
				if (World)
				{
					World->GetTimerManager().SetTimer(RetryTimerHandle, RetryDelegate, RetryDelay, false);
				}
			}
		});

	WebSocket->OnMessage().AddLambda([&](const FString& Message)
		{
			TSharedPtr<FJsonObject> MessageJson;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

			if (!FJsonSerializer::Deserialize(Reader, MessageJson))
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to parse JSON message"));
				return;
			}
			else {
				FString type = MessageJson->GetStringField("type");

					if (type == "DebugInfo") {
						TSharedPtr<FJsonObject> Payload = MessageJson->GetObjectField("data");
						FString DebugMessage = Payload->GetStringField("message");
						GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, *DebugMessage);
					}
					else if (type == "Transcription") {
						FString Transcription = MessageJson->GetStringField("data");
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, *Transcription);
					}
					else if (type == "GenerationStreamStart") {
						// Process Message
						TSharedPtr<FJsonObject> Payload = MessageJson->GetObjectField("data");
						FString CharacterId = Payload->GetStringField("characterId");
						FString ConversationId = Payload->GetStringField("conversationId");

						// Get the NPC pointer
						AStarlightCharacter** Character = CharacterMap.Find(CharacterId);
						if (Character == nullptr) {
							UE_LOG(LogTemp, Warning, TEXT("Character with id %s not found"), *CharacterId)
								return;
						}

						AStarlightNPC* NPC = Cast<AStarlightNPC>(*Character);
						NPC->AudioDecoder->Reset();
						//NPC->AudioComponent->bIsFinishedStreaming = false;

						// Find the conversation this character is in
						UStarlightConversation** ConversationPointer = ConversationMap.Find(ConversationId);
						if (ConversationPointer == nullptr) {
							UE_LOG(LogTemp, Warning, TEXT("Conversation with id %s not found: : GenerationStreamStart"), *ConversationId)
								return;
						}

						UStarlightConversation* Conversation = Cast<UStarlightConversation>(*ConversationPointer);
						if (Conversation == nullptr) {
							UE_LOG(LogTemp, Warning, TEXT("Conversation is not a UStarlightConversation"))
								return;
						}

						// Add NPC to conversation speaker queue
						Conversation->NPCSpeakerQueue.Enqueue(NPC);
					}
					else if (type == "GenerationStreamData") {
						// Process Message
						TSharedPtr<FJsonObject> Payload = MessageJson->GetObjectField("data");
						FString CharacterId = Payload->GetStringField("characterId");
						FString ConversationId = Payload->GetStringField("conversationId");
						FString Base64AudioData = Payload->GetStringField("mp3Bytes");

						// Decode Base64
						TArray<uint8> AudioData;
						FBase64::Decode(Base64AudioData, AudioData);

						// Get the NPC pointer
						AStarlightCharacter** Character = CharacterMap.Find(CharacterId);
						if (Character == nullptr) {
							UE_LOG(LogTemp, Warning, TEXT("Character with id %s not found"), *CharacterId)
								return;
						}

						AStarlightNPC* NPC = Cast<AStarlightNPC>(*Character);

						// Add audio data to NPC's audio decoder
						UE_LOG(LogTemp, Warning, TEXT("Adding audio to %s's audio decoder. [%d]"), *NPC->GetName(), NPC->AudioDecoder->QueuedSamples);
						NPC->AppendAudio(AudioData);

						// Find the conversation this character is in
						UStarlightConversation** ConversationPointer = ConversationMap.Find(ConversationId);
						if (ConversationPointer == nullptr) {
							UE_LOG(LogTemp, Warning, TEXT("Conversation with id %s not found: GenerationStreamData"), *ConversationId)
								return;
						}

						UStarlightConversation* Conversation = Cast<UStarlightConversation>(*ConversationPointer);
						if (Conversation == nullptr) {
							UE_LOG(LogTemp, Warning, TEXT("Conversation is not a UStarlightConversation"))
								return;
						}

						// If the top of the queue is the current NPC, play the sound when applicable

						AStarlightNPC* TopOfQueue;
						Conversation->NPCSpeakerQueue.Peek(TopOfQueue);

						if (Conversation != nullptr && TopOfQueue == NPC && NPC->AudioDecoder->QueuedSamples >= 5000 && !NPC->bIsSpeaking) {
							// -- Error handling --
							if (NPC->AudioDecoder->SoundWave == nullptr) {
								UE_LOG(LogTemp, Warning, TEXT("AudioDecoder.SoundWave is null"))
									return;
							}

							if (NPC->AudioDecoder->SoundWave->IsPlayable() == false) {
								UE_LOG(LogTemp, Warning, TEXT("AudioDecoder.SoundWave is not playable"))
									return;
							}

							// -- Play the sound --
							NPC->Speak();
						}
					}
					else if (type == "GenerationStreamEnd") {
						// Process Message
						TSharedPtr<FJsonObject> Payload = MessageJson->GetObjectField("data");
						FString CharacterId = Payload->GetStringField("characterId");
						FString ConversationId = Payload->GetStringField("conversationId");

						// Get the NPC pointer
						AStarlightCharacter** Character = CharacterMap.Find(CharacterId);
						if (Character == nullptr) {
							UE_LOG(LogTemp, Warning, TEXT("Character with id %s not found"), *CharacterId)
								return;
						}

						AStarlightNPC* NPC = Cast<AStarlightNPC>(*Character);

						// Set AudioDecoder's bIsFinishStreaming to true
						if (NPC->AudioDecoder == nullptr) {
							UE_LOG(LogTemp, Warning, TEXT("AudioDecoder is null when trying to set bIsFinishedStreaming"))
								return;
						}

						UE_LOG(LogTemp, Warning, TEXT("[%s] Setting bIsFinishedStreaming to true"), *NPC->AudioDecoder->SoundWave->GetName());
						// NPC->AudioDecoder->SoundWave->bIsFinishedStreaming = true;

						return;
					}
					else if (type == "QuestGeneration") {
						TSharedPtr<FJsonObject> Payload = MessageJson->GetObjectField("data");
						FString QuestId = Payload->GetStringField("id");
						FString QuestTitle = Payload->GetStringField("title");
						FString QuestObjective = Payload->GetStringField("objective");
						FString QuestDescription = Payload->GetStringField("description");

						// Display Quest Widget
						DisplayQuest(QuestTitle, QuestObjective);
					}
					else {
						UE_LOG(LogTemp, Warning, TEXT("Received unknown websocket event."));
					}

			}
		});

	WebSocket->Connect();
}

void UStarlightGameInstance::DisplayQuest(const FString& QuestTitle, const FString& QuestObjective)
{
	if (QuestWidgetBPClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Displaying Quest Widget"));
		// Create an instance of the StarlightQuestWidget and add it to the viewport
		if (!QuestWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("Creating Quest Widget"));
			QuestWidget = CreateWidget<UStarlightQuestWidget>(this, QuestWidgetBPClass);
			QuestWidget->AddToViewport();
			
		}


		UE_LOG(LogTemp, Warning, TEXT("Setting Quest Widget Visibility to true"))
		QuestWidget->SetQuestVisibility(true);
		QuestWidget->QuestTitle->SetText(FText::FromString(QuestTitle));
		QuestWidget->QuestObjective->SetText(FText::FromString(QuestObjective));
	}
}

void UStarlightGameInstance::HideQuestWidget()
{
	if (QuestWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Setting Quest Widget Visibility to false"))
		QuestWidget->SetQuestVisibility(false);
	}
}

void UStarlightGameInstance::Shutdown()
{
	if (WebSocket->IsConnected())
	{
		WebSocket->Close();
	}

	Super::Shutdown();
}
