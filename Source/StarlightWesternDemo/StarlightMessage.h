// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StarlightMessage.generated.h"

/**
 * 
 */
UCLASS()
class STARLIGHTWESTERNDEMO_API UStarlightMessage : public UObject
{
	GENERATED_BODY()
	

public:
	UStarlightMessage();

	FString Id;
	FString Message;
};
