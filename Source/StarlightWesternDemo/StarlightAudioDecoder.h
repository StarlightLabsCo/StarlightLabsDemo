// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "minimp3.h"
#include <Sound/SoundWaveProcedural.h>

/**
 * 
 */
class STARLIGHTWESTERNDEMO_API StarlightAudioDecoder
{
public:
	StarlightAudioDecoder();
	~StarlightAudioDecoder();

	void Append(TArray<uint8>& data);
	void Empty();

	USoundWaveProcedural* SoundWave;
	int QueuedSamples;

	// Debug
	void SavePCM();
	void SaveMP3();

	TArray<uint8> PCMSamples;
	TArray<uint8> MP3Samples;

private:
	TArray<uint8> AudioBytes;

	mp3dec_t mp3d;
	int16_t PCM[MINIMP3_MAX_SAMPLES_PER_FRAME];
	mp3dec_frame_info_t Info;
};
