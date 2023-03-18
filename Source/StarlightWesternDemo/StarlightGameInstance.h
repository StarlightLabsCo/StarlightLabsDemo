// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "IWebSocket.h"
#include "StarlightAudioDecoder.h"
#include "StarlightQuestWidget.h"
#include "StarlightGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class STARLIGHTWESTERNDEMO_API UStarlightGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	TSharedPtr<IWebSocket> WebSocket;
	StarlightAudioDecoder AudioDecoder;

	// Global character map
	FString PlayerId;
	TMap<FString, class AStarlightCharacter*> CharacterMap;
	TMap<FString, StarlightAudioDecoder> AudioDecoderMap;

	UPROPERTY()
	UStarlightQuestWidget* QuestWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest Widget")
	TSubclassOf<UStarlightQuestWidget> QuestWidgetBPClass;

	UFUNCTION()
	void DisplayQuest(const FString& QuestTitle, const FString& QuestObjective);

	UFUNCTION()
	void HideQuestWidget();
};
