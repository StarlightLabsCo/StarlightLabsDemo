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

	// --- Init Quest Widget ---
	UClass* QuestWidgetBPLoaded = StaticLoadClass(UStarlightQuestWidget::StaticClass(), nullptr, TEXT("/Game/Starlight/Blueprints/BP_StarlightQuestWidget.BP_StarlightQuestWidget_C"));
	if (QuestWidgetBPLoaded)
	{
		UE_LOG(LogTemp, Warning, TEXT("Loaded Quest Widget BP Class"));
		QuestWidgetBPClass = QuestWidgetBPLoaded;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find QuestWidgetBPClass"));
	}

	// -- Hide Quest Widget ---
	HideQuestWidget();

	// --- Audio Capture ---
	if (!FModuleManager::Get().IsModuleLoaded("AudioCapture"))
	{
		FModuleManager::Get().LoadModule("AudioCapture");
	}

	// --- Websockets ---
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

	WebSocket->OnConnectionError().AddLambda([](const FString& Error)
		{
			UE_LOG(LogTemp, Warning, TEXT("Error connecting to WebSocket Server: %s"), *Error);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Error);
		});

	WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean)
		{
			UE_LOG(LogTemp, Warning, TEXT("WebSocket Server closed connection: %d %s %d"), StatusCode, *Reason, bWasClean);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "WebSocket Server closed connection");
		});

	// Message Received
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
				UE_LOG(LogTemp, Warning, TEXT("Received message of type: %s"), *type)

				if (type == "Transcription") {
					FString Transcription = MessageJson->GetStringField("data");
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, *Transcription);
				}
				else if (type == "GenerationStreamStart") {
					// Process Message
					TSharedPtr<FJsonObject> Payload = MessageJson->GetObjectField("data");
					FString CharacterId = Payload->GetStringField("id");

					// Create AudioDecoder or empty existing one
					StarlightAudioDecoder& AudioDecoder = AudioDecoderMap.FindOrAdd(CharacterId);
					AudioDecoder.Empty();
				}
				else if (type == "GenerationStreamData") {
					// Process Message
					TSharedPtr<FJsonObject> Payload = MessageJson->GetObjectField("data");
					FString CharacterId = Payload->GetStringField("id");
					FString Base64AudioData = Payload->GetStringField("mp3Bytes");

					// Decode Base64
					TArray<uint8> AudioData;	
					FBase64::Decode(Base64AudioData, AudioData);

					// Append to AudioDecoder
					StarlightAudioDecoder* AudioDecoder = AudioDecoderMap.Find(CharacterId);
					if (AudioDecoder == nullptr) {
						UE_LOG(LogTemp, Warning, TEXT("AudioDecoder with id %s not found"), *CharacterId)
						return;
					}
					AudioDecoder->Append(AudioData);

					// Play sound if enough samples are queued
					UE_LOG(LogTemp, Warning, TEXT("AudioDecoder.QueuedSamples: %d"), AudioDecoder->QueuedSamples)
					if (AudioDecoder->QueuedSamples >= 5000) {				
						AStarlightCharacter** Character = CharacterMap.Find(CharacterId);
						if (Character == nullptr) {
							UE_LOG(LogTemp, Warning, TEXT("Character with id %s not found"), *CharacterId)
							return;
						}

						AStarlightNPC* NPC = Cast<AStarlightNPC>(*Character);
						if (NPC != nullptr && !NPC->bIsSpeaking)
						{
							if (AudioDecoder->SoundWave == nullptr) {
								UE_LOG(LogTemp, Warning, TEXT("AudioDecoder.SoundWave is null"))
									return;
							}

							if (AudioDecoder->SoundWave->IsPlayable() == false) {
								UE_LOG(LogTemp, Warning, TEXT("AudioDecoder.SoundWave is not playable"))
									return;
							}

							UE_LOG(LogTemp, Warning, TEXT("Playing sound"));
							NPC->Speak(AudioDecoder->SoundWave);
						}

					}
					

				}
				else if (type == "GenerationStreamEnd") {
					return;
				}
				else if (type == "QuestGeneration") {
					TSharedPtr<FJsonObject> Payload = MessageJson->GetObjectField("data");
					FString QuestId = Payload->GetStringField("id");
					FString QuestTitle = Payload->GetStringField("title");
					FString QuestObjective = Payload->GetStringField("objective");
					FString QuestDescription = Payload->GetStringField("description");

					// print to screen debug in purple
					FString QuestString = "Quest ID: " + QuestId + " Title: " + QuestTitle + " Objective: " + QuestObjective;
					GEngine->AddOnScreenDebugMessage(-1, 45.0f, FColor::Purple, *QuestString);
					DisplayQuest(QuestTitle, QuestObjective);
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Received unknown websocket event."));
				}

			}
		});

	// Connect to Websocket Server
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
