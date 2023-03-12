// Fill out your copyright notice in the Description page of Project Settings.


#include "StarlightConversation.h"

UStarlightConversation::UStarlightConversation()
{
	// Generate random UUID for Id
	Id = FGuid::NewGuid().ToString();
}