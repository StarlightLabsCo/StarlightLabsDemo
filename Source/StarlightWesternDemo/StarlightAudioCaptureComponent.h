// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AudioCaptureComponent.h"
#include "StarlightAudioCaptureComponent.generated.h"

class USoundSubmixBase;
class USoundWave;

/**
 * 
 */
UCLASS()
class STARLIGHTWESTERNDEMO_API UStarlightAudioCaptureComponent : public UAudioCaptureComponent
{
	GENERATED_BODY()

public:
	UStarlightAudioCaptureComponent(const FObjectInitializer& ObjectInitializer);

	/** Called when the game starts */
	virtual void BeginPlay() override;

	/** Called every frame */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Called when the game ends */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void StartRecording();
	void StopRecording();

	void StartRecordingChunk();
	void StopRecordingChunk();
	void PeriodocFlush();

private:
	std::atomic<bool> bIsRecording = false;
	std::atomic<bool> bIsRecordingChunk = false;
	TQueue<USoundWave*> AudioCaptureDataQueue;

	bool CheckPeriod(UWorld* World);
	bool IsExpired(UWorld* World);

	float LastChunkTime = 0.0f;
	float Threshold = 0.1f;

	void SendStartStreamPacket();
	void SendAudioToWebsocket(USoundWave* SoundWave);
	void SendEndStreamPacket();
};
