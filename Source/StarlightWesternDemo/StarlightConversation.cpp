// Fill out your copyright notice in the Description page of Project Settings.


#include "StarlightConversation.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "StarlightNPC.h"
#include "StarlightGameInstance.h"


UStarlightConversation::UStarlightConversation() {
    // Generate random UUID for Id
    Id = FGuid::NewGuid().ToString();
}

void UStarlightConversation::OnNPCSpeakerFinish() {
    UE_LOG(LogTemp, Warning, TEXT("OnNPCSpeakerFinish()"));

    // Debug
    AStarlightNPC* Speaker;
    NPCSpeakerQueue.Peek(Speaker);
    UE_LOG(LogTemp, Warning, TEXT("Removing %s from queue"), *Speaker->GetName());

    // Pop the prior speaker from the queue
    NPCSpeakerQueue.Pop();

    if (NPCSpeakerQueue.IsEmpty()) {
		return;
	}

    // Get starlight game instance
    if (GameInstance == nullptr) {
        UE_LOG(LogTemp, Warning, TEXT("GameInstance not found"));
		return;
	}

    // Get the next speaker
    AStarlightNPC* NextSpeaker;
    NPCSpeakerQueue.Peek(NextSpeaker);
    if (!NextSpeaker) {
        UE_LOG(LogTemp, Warning, TEXT("NextSpeaker not found"));
        return;
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("NextSpeaker is %s"), *NextSpeaker->GetName());
    }

    // Trigger speak on next speaker
    if (NextSpeaker && NextSpeaker->AudioDecoder->QueuedSamples > 5000 && !NextSpeaker->bIsSpeaking) {
        UE_LOG(LogTemp, Warning, TEXT("Trigger speak on next speaker: %s"), *NextSpeaker->GetName())
		NextSpeaker->Speak();
	}
}