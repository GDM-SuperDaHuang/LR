// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Nav/LrNavMovementComponent.h"

void ULrNavMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementState.bCanCrouch = false;
	MovementState.bCanSwim = false;
	NavMovementProperties.bStopMovementAbortPaths = true;
	NavMovementProperties.bUpdateNavAgentWithOwnersCollision = true;
	SetComponentTickEnabled(true);
}
