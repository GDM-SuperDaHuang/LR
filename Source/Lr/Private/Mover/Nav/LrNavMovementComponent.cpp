// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Nav/LrNavMovementComponent.h"

#include "Component/LrAnimationComponent.h"
#include "Engine/World.h"
#include "Mover/LrMoverComponent.h"
#include "Pawn/LrEnemyPawn.h"

void ULrNavMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	LrEnemyPawn = Cast<ALrEnemyPawn>(GetOwner());
	NavAgentProps.AgentRadius = 100;
	NavAgentProps.AgentHeight = 176;
	NavAgentProps.bCanWalk = true;
	NavMovementProperties.bUpdateNavAgentWithOwnersCollision = true;
}

void ULrNavMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	Super::RequestDirectMove(MoveVelocity, bForceMaxSpeed);
	if (!LrEnemyPawn)
	{
		return;
	}
	if (LrEnemyPawn->LrAnimationComponent->MovementData.Status == 1)
	{
		LrEnemyPawn->LrMoverComponent;
		LrEnemyPawn->UpdateMove(FVector::ZeroVector);
		return;
	}
	// 转换成方向输入
	const FVector MoveDirection = MoveVelocity.GetSafeNormal();
	LrEnemyPawn->UpdateMove(MoveDirection);
	// 面向移动方向
	LrEnemyPawn->FaceToDirection(MoveDirection, GetWorld()->GetDeltaSeconds());
}

void ULrNavMovementComponent::StopActiveMovement()
{
	Super::StopActiveMovement();
	if (!LrEnemyPawn)
	{
		return;
	}
	// 转换成方向输入
	LrEnemyPawn->UpdateMove(FVector::ZeroVector);
}
