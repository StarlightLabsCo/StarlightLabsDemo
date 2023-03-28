// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "minimp3.h"
#include "UObject/NoExportTypes.h"
#include "Sound/SoundWaveProcedural.h"
#include "StarlightAudioDecoder.generated.h"

UCLASS()
class STARLIGHTWESTERNDEMO_API UStarlightAudioDecoder : public UObject
{
	GENERATED_BODY()

public:
	UStarlightAudioDecoder();
	void Init();

	~UStarlightAudioDecoder();

	void Append(TArray<uint8>& data);
	void Reset();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Starlight")
	TObjectPtr<USoundWaveProcedural> SoundWave;
	int QueuedSamples;

private:
	TArray<uint8> AudioBytes;

	mp3dec_t mp3d;
	int16_t PCM[MINIMP3_MAX_SAMPLES_PER_FRAME];
	mp3dec_frame_info_t Info;
};
