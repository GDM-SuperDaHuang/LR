// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/LrCharacterMoverComponent.h"

#include "Backends/MoverStandaloneLiaison.h"

ULrCharacterMoverComponent::ULrCharacterMoverComponent()
{
	BackendClass = UMoverStandaloneLiaisonComponent::StaticClass();
}

void ULrCharacterMoverComponent::OnRegister()
{
	Super::OnRegister();
	// REQUIRED: Otherwise, CommonLegacyMovementSettings does not get initialized prior to the MovementMode objects.
	RefreshSharedSettings();
}
