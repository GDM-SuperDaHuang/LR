// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "LrBaseMovementMode.generated.h"

USTRUCT()
struct FLrMoverInputCmd : public FMoverInputCmdContext
{
	GENERATED_BODY()

	UPROPERTY()
	FVector2D MoveInput = FVector2D::ZeroVector;

	uint8 bJumpPressed : 1;
	uint8 bDashPressed : 1;

	FLrMoverInputCmd()
		: bJumpPressed(false)
		, bDashPressed(false)
	{}
};

USTRUCT()
struct FLrMoverSyncState : public FMoverSyncState
{
	GENERATED_BODY()

	// ===== 地面状态 =====
	UPROPERTY()
	bool bIsOnGround = true;

	// ===== Jump 状态 =====
	UPROPERTY()
	int32 JumpCount = 0;

	UPROPERTY()
	bool bJumpConsumed = false;

	// ===== Dash 状态 =====
	UPROPERTY()
	float DashTimeRemaining = 0.f;

	UPROPERTY()
	FVector DashDirection = FVector::ZeroVector;
};
/**
 * 
 */
UCLASS()
class LR_API ULrBaseMovementMode : public UBaseMovementMode
{
	GENERATED_BODY()
	
};
