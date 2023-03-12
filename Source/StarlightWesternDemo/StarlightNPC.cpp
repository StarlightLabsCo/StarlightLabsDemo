// Fill out your copyright notice in the Description page of Project Settings.


#include "StarlightNPC.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "StarlightGameInstance.h"

// Sets default values
AStarlightNPC::AStarlightNPC()
{
	// Setup Audio
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->AttenuationSettings = LoadObject<USoundAttenuation>(nullptr, TEXT("/Game/Starlight/Sound/NPCAttenuation.NPCAttenuation"));

	// Create a delegate to set bIsSpeaking to false when the sound is finished playing
	AudioComponent->OnAudioFinished.AddDynamic(this, &AStarlightNPC::OnAudioFinished);

	bIsSpeaking = false;

	return;
}

void AStarlightNPC::Speak(USoundWave* NewSoundWave)
{
	bIsSpeaking = true;
	AudioComponent->SetSound(NewSoundWave);
	AudioComponent->Play();

	UE_LOG(LogTemp, Warning, TEXT("Playing sound %s"), *NewSoundWave->GetName());
	UE_LOG(LogTemp, Warning, TEXT("NewSoundWave Length: %d"), NewSoundWave->RawPCMDataSize);	
}

void AStarlightNPC::OnAudioFinished()
{
	bIsSpeaking = false;
}


// Called when the game starts or when spawned
void AStarlightNPC::BeginPlay()
{
	Super::BeginPlay();	
}



