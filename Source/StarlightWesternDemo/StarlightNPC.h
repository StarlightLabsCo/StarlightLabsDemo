// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StarlightCharacter.h"
#include "StarlightNPC.generated.h"

class UStarlightAudioComponent;
class USoundWave;

UCLASS()
class STARLIGHTWESTERNDEMO_API AStarlightNPC : public AStarlightCharacter
{
	GENERATED_BODY()

	/** Audio capture component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Audio, meta = (AllowPrivateAccess = "true"))
	UStarlightAudioComponent* AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio, meta = (AllowPrivateAccess = "true"))
	USoundAttenuation* NPCAttenuation;

	UFUNCTION()
	void OnAudioFinished();

public:
	AStarlightNPC();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Starlight")
	class UStarlightAudioDecoder* AudioDecoder;

	void AppendAudio(TArray<uint8>& data);
	void Speak();
	bool bIsSpeaking = false;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
