// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LrAnimInstance.h"

#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "Lib/LrCommonLibrary.h"
#include "Mover/LrMoverComponent.h"

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

	ULrMoverComponent* Mover = Pawn->FindComponentByClass<ULrMoverComponent>();

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
