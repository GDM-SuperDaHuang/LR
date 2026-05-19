// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LrAnimationComponent.h"

#include "KismetAnimationLibrary.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMoverComponent.h"
#include "Pawn/LrPawnBase.h"

// Sets default values for this component's properties
ULrAnimationComponent::ULrAnimationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void ULrAnimationComponent::BeginPlay()
{
	Super::BeginPlay();
	CachedPawn = Cast<ALrPawnBase>(GetOwner());
	CachedPawn = GetOwner<ALrPawnBase>();

	if (CachedPawn)
	{
		CachedMover = CachedPawn->FindComponentByClass<ULrMoverComponent>();
	}
}


// Called every frame
void ULrAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateMovementData(DeltaTime);
}

void ULrAnimationComponent::UpdateMovementData(float DeltaTime)
{
	if (!CachedPawn || !CachedMover)
	{
		return;
	}


	MovementData.Velocity = CachedMover->GetVelocity();
	//---------------------------------------------------
	// Speed
	//---------------------------------------------------
	MovementData.Speed = MovementData.Velocity.Size();

	//---------------------------------------------------
	// Direction
	//---------------------------------------------------
	MovementData.Direction = UKismetAnimationLibrary::CalculateDirection(
		MovementData.Velocity,
		CachedPawn->GetActorRotation()
	);

	FName ModeName = CachedMover->GetMovementModeName();
	MovementData.bIsBlink = ModeName.IsEqual(LrAllModes::Blink);
	MovementData.bIsFalling = ModeName.IsEqual(LrAllModes::Air);
	// if (MovementData.bIsFalling)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[UpdateMovementData] CachedMover=%p "), CachedMover.Get());
	// }
	MovementData.bIsJumping = CachedMover->bJumpInitiated;
	// if (MovementData.bIsJumping)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[UpdateMovementData] CachedMover=%p "), CachedMover.Get());
	// }
}
