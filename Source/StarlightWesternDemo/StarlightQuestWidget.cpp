// Fill out your copyright notice in the Description page of Project Settings.


#include "StarlightQuestWidget.h"
#include "Components/TextBlock.h"

void UStarlightQuestWidget::NativeConstruct()
{
	Super::NativeConstruct();

	QuestTitle = Cast<UTextBlock>(GetWidgetFromName(TEXT("QuestTitle")));
	QuestObjective = Cast<UTextBlock>(GetWidgetFromName(TEXT("QuestObjective")));
}

void UStarlightQuestWidget::SetQuestVisibility(bool bVisible)
{
	ESlateVisibility NewVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	SetVisibility(NewVisibility);
}