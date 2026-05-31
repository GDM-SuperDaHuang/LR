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
	PrimaryComponentTick.bCanEverTick = false;
	// SetComponentTickInterval(0.3f);
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
// void ULrAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
// {
// 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
// 	UpdateMovementData(DeltaTime);
// }

// FLrAnimMovementData& ULrAnimationComponent::UpdateMovementData()
// {
// 	if (!CachedPawn || !CachedMover)
// 	{
// 		return MovementData;
// 	}
//
// 	// 保存旧数据用于对比
// 	// PreviousMovementData = MovementData;
//
// 	MovementData.Velocity = CachedMover->GetVelocity();
// 	//---------------------------------------------------
// 	// Speed
// 	//---------------------------------------------------
// 	MovementData.Speed = MovementData.Velocity.Size();
//
// 	//---------------------------------------------------
// 	// Direction
// 	//---------------------------------------------------
// 	MovementData.Direction = UKismetAnimationLibrary::CalculateDirection(
// 		MovementData.Velocity,
// 		CachedPawn->GetActorRotation()
// 	);
//
// 	FName ModeName = CachedMover->GetMovementModeName();
// 	MovementData.bIsBlink = ModeName.IsEqual(LrAllModes::Blink);
// 	MovementData.bIsFalling = ModeName.IsEqual(LrAllModes::Air);
// 	// if (MovementData.bIsFalling)
// 	// {
// 	// 	UE_LOG(LogTemp, Warning, TEXT("[UpdateMovementData] CachedMover=%p "), CachedMover.Get());
// 	// }
// 	MovementData.bIsJumping = CachedMover->bJumpInitiated;
// 	// if (MovementData.bIsJumping)
// 	// {
// 	// 	UE_LOG(LogTemp, Warning, TEXT("[UpdateMovementData] CachedMover=%p "), CachedMover.Get());
// 	// }
//
// 	// 数据有显著变化时才广播，避免无效通知
// 	// if (HasDataChangedSignificantly(PreviousMovementData))
// 	// {
// 	// 	OnMovementDataChanged.ExecuteIfBound(MovementData);
// 	// }
// 	return MovementData;
// }

// bool ULrAnimationComponent::HasDataChangedSignificantly()
// {
// 	// 速度变化 
// 	if (MovementData.Speed > 0.5 && PreviousMovementData.Speed < 0.5) //0~0.5的时候
// 	{
// 		PreviousMovementData.Speed = MovementData.Speed;
// 		return true;
// 	}
// 	else if (MovementData.Speed < 0.5f && PreviousMovementData.Speed > 0.5) //回到0~0.5的时候
// 	{
// 		PreviousMovementData.Speed = MovementData.Speed;
// 		return true;
// 	}
//
// 	// 方向变化 > 1 度认为有变化
// 	// if (FMath::Abs(MovementData.Direction - PreviousMovementData.Direction) > 1.0f)
// 	// {
// 	// 	return true;
// 	// }
//
// 	// 状态位变化
// 	if (MovementData.bIsJumping != PreviousMovementData.bIsJumping ||
// 	    MovementData.bIsBlink != PreviousMovementData.bIsBlink ||
// 	    MovementData.bIsFalling != PreviousMovementData.bIsFalling ||
// 	    MovementData.bIsCrouching != PreviousMovementData.bIsCrouching)
// 	{
// 		return true;
// 	}
//
// 	return false;
// }
