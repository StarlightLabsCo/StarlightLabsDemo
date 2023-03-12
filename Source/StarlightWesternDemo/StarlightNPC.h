// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StarlightCharacter.h"
#include "StarlightNPC.generated.h"

class UAudioComponent;
class USoundWave;

UCLASS()
class STARLIGHTWESTERNDEMO_API AStarlightNPC : public AStarlightCharacter
{
	GENERATED_BODY()

	/** Audio capture component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Audio, meta = (AllowPrivateAccess = "true"))
	UAudioComponent* AudioComponent;

public:
	// Sets default values for this character's properties
	AStarlightNPC();

	void Speak(USoundWave* NewSoundWave);
	bool bIsSpeaking;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	USoundWave* SoundWave;
	void OnAudioFinished();

};
