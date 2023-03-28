// Fill out your copyright notice in the Description page of Project Settings.


#include "StarlightSoundWaveProcedural.h"
#include "Components/AudioComponent.h"


int32 UStarlightSoundWaveProcedural::GeneratePCMData(uint8* PCMData, const int32 SamplesNeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] Need %d PCM Samples"), *this->GetName() , SamplesNeeded);
	int32 BytesAvailable = GetAvailableAudioByteCount();
	if (bIsFinishedStreaming && BytesAvailable <= 0) {
		UE_LOG(LogTemp, Warning, TEXT("[%s] No more PCM Samples"), *this->GetName());
	}
	

	int32 tempValue = Super::GeneratePCMData(PCMData, SamplesNeeded);
	UE_LOG(LogTemp, Warning, TEXT("[%s] Generated %d PCM Samples"), *this->GetName(), tempValue);
	return tempValue;
}
