// Fill out your copyright notice in the Description page of Project Settings.


#include "StarlightNPC.h"
#include "StarlightAudioComponent.h"
#include "Sound/SoundWave.h"
#include "StarlightAudioDecoder.h"
#include "StarlightGameInstance.h"

// Sets default values
AStarlightNPC::AStarlightNPC()
{
	// Attach audio component
	AudioComponent = CreateDefaultSubobject<UStarlightAudioComponent>(TEXT("StarlightAudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);

	bIsSpeaking = false;

	return;
}

void AStarlightNPC::BeginPlay() {
	Super::BeginPlay();

	// Attach Attenuation Settings
	AudioComponent->AttenuationSettings = NPCAttenuation;

	// Check if the delegate is already bound
	AudioComponent->OnAudioFinished.AddUniqueDynamic(this, &AStarlightNPC::OnAudioFinished);

	// AudioDecoder
	AudioDecoder = NewObject<UStarlightAudioDecoder>();
	AudioDecoder->Init();	
}

void AStarlightNPC::AppendAudio(TArray<uint8>& data) {
	if (AudioDecoder == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("AudioDecoder is null"))
			return;
	}

	AudioDecoder->Append(data);
}

void AStarlightNPC::Speak()
{
	bIsSpeaking = true;
	AudioComponent->SetSound(AudioDecoder->SoundWave);
	AudioComponent->Play();
}

void AStarlightNPC::OnAudioFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("OnAudioFinished"));

	bIsSpeaking = false;

	if (ActiveConversation != nullptr) {
		ActiveConversation->OnNPCSpeakerFinish();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("CurrentConversation is null"));
	}
}



