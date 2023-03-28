// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StarlightConversation.generated.h"

/**
 * 
 */
UCLASS()
class STARLIGHTWESTERNDEMO_API UStarlightConversation : public UObject
{
	GENERATED_BODY()

public:
	UStarlightConversation();

	FString Id;

	TArray<class AStarlightCharacter*> Participants;
	TQueue<class AStarlightNPC*> NPCSpeakerQueue;
	void OnNPCSpeakerFinish();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Starlight")
	class UStarlightGameInstance* GameInstance;
};
