// Fill out your copyright notice in the Description page of Project Settings.


#include "StarlightAudioCaptureComponent.h"
#include "Sound/SoundSubmix.h"
#include <AudioMixerBlueprintLibrary.h>
#include "StarlightGameInstance.h"
#include "StarlightPlayer.h"
#include "StarlightCharacter.h"
#include "StarlightConversation.h"

UStarlightAudioCaptureComponent::UStarlightAudioCaptureComponent(const FObjectInitializer& ObjectInitializer) : Super (ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = true;

	// Connect Submix to AudioCaptureComponent so that players don't hear themselves
	SoundSubmix = LoadObject<USoundSubmixBase>(nullptr, TEXT("/Game/Starlight/Sound/MuteMicSubmix.MuteMicSubmix"));
}

void UStarlightAudioCaptureComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStarlightAudioCaptureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TickType == ELevelTick::LEVELTICK_PauseTick && bIsRecordingChunk)
	{
		StopRecordingChunk();
		return;
	}

	if (bIsRecording && !bIsRecordingChunk) {
		StartRecordingChunk();
	}
	
	PeriodocFlush();
}

void UStarlightAudioCaptureComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UStarlightAudioCaptureComponent::StartRecording()
{
	if (bIsRecording) {
		return;
	}

	bIsRecording = true;

	SendStartStreamPacket();

	// Start recording
	Start();
}

void UStarlightAudioCaptureComponent::StopRecording()
{
	if (!bIsRecording) {
		return;
	}

	// Stop recording
	Stop();

	// Clear out data queue if needed
	USoundWave* NextAudioCaptureChunk;
	while (AudioCaptureDataQueue.Dequeue(NextAudioCaptureChunk)) {
		SendAudioToWebsocket(NextAudioCaptureChunk);
	}

	SendEndStreamPacket();

	bIsRecording = false;
}

void UStarlightAudioCaptureComponent::StartRecordingChunk()
{
	if (bIsRecordingChunk) {
		return;
	}

	bIsRecordingChunk = true;

	UAudioMixerBlueprintLibrary::StartRecordingOutput(this, Threshold, Cast<USoundSubmix>(this->SoundSubmix));
}

void UStarlightAudioCaptureComponent::StopRecordingChunk()
{
	if (!bIsRecordingChunk) {
		return;
	}

	USoundWave* SoundWaveMic = UAudioMixerBlueprintLibrary::StopRecordingOutput(this, EAudioRecordingExportType::SoundWave, "VoiceRecording", "VoiceRecording", Cast<USoundSubmix>(this->SoundSubmix));

	AudioCaptureDataQueue.Enqueue(SoundWaveMic);

	bIsRecordingChunk = false;
}

void UStarlightAudioCaptureComponent::PeriodocFlush() {
	if (bIsRecording && CheckPeriod(GetWorld())) {
		FAudioThread::RunCommandOnAudioThread([this]()
			{
				StopRecordingChunk();
				StartRecordingChunk();
			});
	}

	USoundWave* NextAudioCaptureChunk;
	if (AudioCaptureDataQueue.Dequeue(NextAudioCaptureChunk)) {
		SendAudioToWebsocket(NextAudioCaptureChunk);
	}
}

bool UStarlightAudioCaptureComponent::CheckPeriod(UWorld* World)
{
	if (IsExpired(World))
	{
		LastChunkTime = World->GetTimeSeconds();
		return true;
	}
	return false;
}

bool UStarlightAudioCaptureComponent::IsExpired(UWorld* World)
{
	return World->GetTimeSeconds() > LastChunkTime + Threshold;
}

void UStarlightAudioCaptureComponent::SendStartStreamPacket() {
	AStarlightPlayer* Player = Cast<AStarlightPlayer>(GetOwner());
	UStarlightGameInstance* GameInstance = Cast<UStarlightGameInstance>(Player->GetGameInstance());
	if (!GameInstance || !GameInstance->WebSocket->IsConnected()) {
		return;
	}

	FString Packet = FString::Printf(TEXT("{\"type\":\"AudioCaptureStreamStart\",\"data\":{\"playerId\":\"%s\"}}"), *Player->Id);
	GameInstance->WebSocket->Send(Packet);
}

void UStarlightAudioCaptureComponent::SendEndStreamPacket() {
	AStarlightPlayer* Player = Cast<AStarlightPlayer>(GetOwner());
	if (!Player) {
		UE_LOG(LogTemp, Warning, TEXT("Player is null"));
		return;
	}

	UStarlightGameInstance* GameInstance = Cast<UStarlightGameInstance>(Player->GetGameInstance());
	if (!GameInstance || !GameInstance->WebSocket->IsConnected()) {
		return;
	}

	/// get active conversation
	UStarlightConversation* Conversation = Player->ActiveConversation;
	if (!Conversation) {
		UE_LOG(LogTemp, Warning, TEXT("Conversation is null"));
		return;
	}

	// get the character that the player is actively talking to
	auto Participants = Conversation->Participants;
	if (Participants.Num() <= 0) {
		UE_LOG(LogTemp, Warning, TEXT("Participants is empty"));
		return;
	}

	// get the participant that isn't the player
	AStarlightCharacter* TargetCharacter = nullptr;
	for (auto Participant : Participants) {
		if (Participant != Player) {
			TargetCharacter = Participant;
			break;
		}
	}

	if (!TargetCharacter) {
		UE_LOG(LogTemp, Warning, TEXT("TargetCharacter is null"));
		return;
	}

	FString Packet = FString::Printf(TEXT("{\"type\":\"AudioCaptureStreamEnd\",\"data\":{\"playerId\":\"%s\"}}"), *Player->Id);
	GameInstance->WebSocket->Send(Packet);
}



void UStarlightAudioCaptureComponent::SendAudioToWebsocket(USoundWave* SoundWave) {
	AStarlightPlayer* Player = Cast<AStarlightPlayer>(GetOwner());
	UStarlightGameInstance* GameInstance = Cast<UStarlightGameInstance>(GetOwner()->GetGameInstance());
	if (!GameInstance || !GameInstance->WebSocket->IsConnected()) {
		return;
	}

	if (!SoundWave) {
		UE_LOG(LogTemp, Warning, TEXT("SoundWave is null"));
		return;
	}

	uint8* RawPCMData = SoundWave->RawPCMData;
	if (!RawPCMData) {
		UE_LOG(LogTemp, Warning, TEXT("RawPCMData is null"));
		return;
	}

	int32 RawPCMDataSize = SoundWave->RawPCMDataSize;
	if (RawPCMDataSize <= 0) {
		UE_LOG(LogTemp, Warning, TEXT("RawPCMDataSize is null"));
		return;
	}

	FString EncodedString = FBase64::Encode((uint8*)RawPCMData, RawPCMDataSize);
	FString Packet = FString::Printf(TEXT("{\"type\":\"AudioCaptureStreamData\",\"data\":{\"playerId\":\"%s\",\"buffer\":\"%s\"}}"), *Player->Id, *EncodedString);
	GameInstance->WebSocket->Send(Packet);
}