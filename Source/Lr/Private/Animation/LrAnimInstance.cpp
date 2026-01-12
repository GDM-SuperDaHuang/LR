// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LrAnimInstance.h"

#include "DefaultMovementSet/CharacterMoverComponent.h"

void ULrAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
	{
		Speed = 0.f;
		bIsMoving = false;
		return;
	}

	UCharacterMoverComponent* Mover = Pawn->FindComponentByClass<UCharacterMoverComponent>();

	if (!Mover)
	{
		Speed = 0.f;
		bIsMoving = false;
		return;
	}

	VelocityWS = Mover->GetVelocity();
	Speed = VelocityWS.Size();
	bIsMoving = Speed > 3.f;
}
