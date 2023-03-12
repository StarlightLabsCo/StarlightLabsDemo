// Copyright Epic Games, Inc. All Rights Reserved.

#include "StarlightCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "StarlightGameInstance.h"


//////////////////////////////////////////////////////////////////////////
// AStarlightDemoCharacter

AStarlightCharacter::AStarlightCharacter()
{
	// Generate random UUID for Id
	Id = FGuid::NewGuid().ToString();
}

void AStarlightCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Get game instance and get the global character map
	UStarlightGameInstance* GameInstance = Cast<UStarlightGameInstance>(this->GetGameInstance());
	if (GameInstance)
	{
		// Add this character to the global character map
		GameInstance->CharacterMap.Add(Id, this);
	}
}




