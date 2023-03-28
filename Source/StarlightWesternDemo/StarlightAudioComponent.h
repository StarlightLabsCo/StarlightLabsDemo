// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "StarlightAudioComponent.generated.h"

/**
 * 
 */
UCLASS()
class STARLIGHTWESTERNDEMO_API UStarlightAudioComponent : public UAudioComponent
{
	GENERATED_BODY()

public:
	bool bIsFinishedStreaming = false;
	
};
