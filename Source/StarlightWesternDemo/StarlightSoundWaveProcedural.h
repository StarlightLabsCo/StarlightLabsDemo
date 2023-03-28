// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundWaveProcedural.h"
#include "StarlightSoundWaveProcedural.generated.h"

/**
 * 
 */
UCLASS()
class STARLIGHTWESTERNDEMO_API UStarlightSoundWaveProcedural : public USoundWaveProcedural
{
	GENERATED_BODY()

	public:
		bool bIsFinishedStreaming = false;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Starlight")
		TObjectPtr<UAudioComponent> AudioComponent;
		int underflowCount = 0;

		virtual int32 GeneratePCMData(uint8* PCMData, const int32 SamplesNeeded) override;
};
