// Copyright Epic Games, Inc. All Rights Reserved.

#include "StarlightGameMode.h"
#include "StarlightCharacter.h"
#include "UObject/ConstructorHelpers.h"

AStarlightGameMode::AStarlightGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Starlight/Blueprints/BP_StarlightPlayer"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
