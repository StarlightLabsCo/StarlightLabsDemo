// Fill out your copyright notice in the Description page of Project Settings.

#define MINIMP3_ONLY_MP3
#define MINIMP3_IMPLEMENTATION

#include "StarlightAudioDecoder.h"

StarlightAudioDecoder::StarlightAudioDecoder()
{
	mp3dec_init(&mp3d);
	SoundWave = NewObject<USoundWaveProcedural>();
	SoundWave->NumChannels = 1;
	SoundWave->SetSampleRate(44100);

	QueuedSamples = 0;
}

StarlightAudioDecoder::~StarlightAudioDecoder()
{
}

void StarlightAudioDecoder::Append(TArray<uint8>& data) {

	// Buffer the data
	AudioBytes.Append(data);
	//MP3Samples.Append(data); // Debug: data saved to debug MP3 file

	int samples;

	// Decoding loop
	while (true) {
		if (AudioBytes.Num() < 1000) {
			// Not enough data in the buffer to decode a frame
			break;
		}

		samples = mp3dec_decode_frame(&mp3d, AudioBytes.GetData(), AudioBytes.Num(), PCM, &Info);

		if (samples <= 0) {
			if (Info.frame_bytes > 0) {
				//UE_LOG(LogTemp, Warning, TEXT("The decoder skipped ID3 or invalid data"));
				break;
			}
			else if (Info.frame_bytes == 0) {
				//UE_LOG(LogTemp, Warning, TEXT("Insufficient data"));
				break;
			}
		}

		///UE_LOG(LogTemp, Warning, TEXT("Decoded %d frame bytes"), Info.frame_bytes);

		// Remove the decoded data from the buffer
		AudioBytes.RemoveAt(0, Info.frame_bytes, true);
		//UE_LOG(LogTemp, Warning, TEXT("Deleted %d bytes from AudioBytes"), Info.frame_bytes);

		// Queue the decoded data to the sound wave
		SoundWave->QueueAudio((uint8*)PCM, samples * 2);
		//PCMSamples.Append((uint8*)PCM, samples * 2); // Debug: Samples saved to debug PCM file
		
		QueuedSamples += samples;
	}
}

void StarlightAudioDecoder::Empty() {
	AudioBytes.Empty();
	SoundWave->ResetAudio();
	QueuedSamples = 0;

	//PCMSamples.Empty(); //Debug
	//MP3Samples.Empty(); //Debug
}

void StarlightAudioDecoder::SaveMP3()
{
	// Save PCMSamples to file
	FILE* f = fopen("C:\\Users\\harri\\Desktop\\test.mp3", "wb");
	if (f)
	{
		fwrite(MP3Samples.GetData(), sizeof(uint8), MP3Samples.Num(), f);
		fclose(f);
	}
	UE_LOG(LogTemp, Warning, TEXT("[MP3] Saved %d samples to file"), PCMSamples.Num());
}

void StarlightAudioDecoder::SavePCM()
{
	// Save PCMSamples to file
	FILE* f = fopen("C:\\Users\\harri\\Desktop\\test.pcm", "wb");
	if (f)
	{
		fwrite(PCMSamples.GetData(), sizeof(uint8), PCMSamples.Num(), f);
		fclose(f);
	}
	UE_LOG(LogTemp, Warning, TEXT("[WAV] Saved %d samples to file"), PCMSamples.Num());
}
