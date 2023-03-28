// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "StarlightConversation.h"
#include "StarlightCharacter.generated.h"

UCLASS(config=Game)
class AStarlightCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AStarlightCharacter();

	UPROPERTY(EditAnywhere, Category = "Starlight")
	FString Id;

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Starlight")
	UStarlightConversation* ActiveConversation;
};

