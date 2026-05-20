// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Nav/LrNavMovementComponent.h"

#include "Mover/LrMoverComponent.h"
#include "Pawn/LrEnemyPawn.h"

void ULrNavMovementComponent::BeginPlay()
{
	LrEnemyPawn = Cast<ALrEnemyPawn>(GetOwner());
	NavAgentProps.AgentRadius = 34.f;
	NavAgentProps.AgentHeight = 176;
	NavAgentProps.bCanWalk = true;
	NavMovementProperties.bUpdateNavAgentWithOwnersCollision = true;
	
	// Super::BeginPlay();
	// LrEnemyPawn = Cast<ALrPawnBase>(GetOwner());
	// NavAgentProps.AgentRadius = 34.f;
	// NavAgentProps.AgentHeight = 88.f;
	// NavAgentProps.bCanWalk = true;
	// NavAgentProps.bCanCrouch = false;
	// NavAgentProps.bCanJump = false;
	// NavAgentProps.bCanSwim = false;
	// NavMovementProperties.bUseAccelerationForPaths =false;
	//
	// // PrimaryComponentTick.bCanEverTick = false;
	// MovementState.bCanCrouch = false;
	// MovementState.bCanSwim = false;
	// NavMovementProperties.bStopMovementAbortPaths = true;
	// NavMovementProperties.bUpdateNavAgentWithOwnersCollision = true;
	// SetComponentTickEnabled(true);
	// SetUpdatedComponent(Cast<USceneComponent>(LrEnemyPawn->GetRootComponent()));
}

void ULrNavMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	if (!LrEnemyPawn)
	{
		return;
	}
	// 转换成方向输入
	const FVector MoveDirection = MoveVelocity.GetSafeNormal();
	LrEnemyPawn->UpdateMove(MoveDirection);
	// 面向移动方向
	LrEnemyPawn->FaceToDirection(MoveDirection,GetWorld()->GetDeltaSeconds());
	// Super::RequestDirectMove(MoveVelocity, bForceMaxSpeed);
}

