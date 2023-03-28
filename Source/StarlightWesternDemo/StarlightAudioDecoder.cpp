// Fill out your copyright notice in the Description page of Project Settings.

#define MINIMP3_ONLY_MP3
#define MINIMP3_IMPLEMENTATION

#include "StarlightAudioDecoder.h"

UStarlightAudioDecoder::UStarlightAudioDecoder() {

}

void UStarlightAudioDecoder::Init() {
	mp3dec_init(&mp3d);

	SoundWave = NewObject<USoundWaveProcedural>();
	UE_LOG(LogTemp, Warning, TEXT("Sound Wave Created: %s"), *SoundWave->GetName());

	SoundWave->NumChannels = 1;
	SoundWave->SetSampleRate(44100);

	QueuedSamples = 0;

	Reset();
}

UStarlightAudioDecoder::~UStarlightAudioDecoder()
{
	
}

void UStarlightAudioDecoder::Append(TArray<uint8>& data){
	// Buffer the data
	AudioBytes.Append(data);

	int samples;

	// Decoding loop
	while (true) {
		if (AudioBytes.Num() < 1000) {
			// Not enough data in the buffer to decode a frame
			break;
		}

		samples = mp3dec_decode_frame(&mp3d, AudioBytes.GetData(), AudioBytes.Num(), PCM, &Info);

		if (samples <= 0 && Info.frame_bytes >= 0) {
			break;
		}

		// Remove the decoded data from the buffer
		AudioBytes.RemoveAt(0, Info.frame_bytes, true);

		if (SoundWave == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("SoundWave is null"))
			return;
		}

		// Queue the decoded data to the sound wave
		SoundWave->QueueAudio((uint8*)PCM, samples * 2);
		
		QueuedSamples += samples;
	}
}

void UStarlightAudioDecoder::Reset() {
	AudioBytes.Empty();
	SoundWave->ResetAudio();
	QueuedSamples = 0;
}