// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StarlightQuestWidget.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class STARLIGHTWESTERNDEMO_API UStarlightQuestWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetQuestVisibility(bool bVisible);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* QuestTitle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* QuestObjective;

protected:
	virtual void NativeConstruct() override;
};
