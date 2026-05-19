// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LrAnimInstance.h"

#include "Component/LrAnimationComponent.h"
#include "Mover/LrMoverComponent.h"

void ULrAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
	{
		return;
	}
	AnimationComponent = Pawn->FindComponentByClass<ULrAnimationComponent>();
}

void ULrAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!AnimationComponent)
	{
		return;
	}
	MovementData = AnimationComponent->GetMovementData();
}
