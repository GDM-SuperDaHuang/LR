// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "LrWalkMovementMode.generated.h"

struct FFloorCheckResult;
class ULrMoverComponent;
/**
 * 
 */
UCLASS()
class LR_API ULrWalkMovementMode : public UBaseMovementMode
{
	GENERATED_BODY()

public:
	ULrWalkMovementMode();
	virtual void Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState) override;
	virtual void GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;

private:
	UPROPERTY()
	TObjectPtr<ULrMoverComponent> CacheMoverComponent;

	// // 当前加速进度（0.0 ~ 1.0），用于启动/大角度转向时的速度惩罚
	// mutable float CurrentAccelerationRamp = 1.0f;
	// // 上一帧的输入方向，用于检测方向变化
	// mutable FVector LastMoveIntent = FVector::ZeroVector;
	// // 上一帧的计算速度，用于避免 SyncState 速度为 0 的问题
	// mutable FVector LastCalculatedVelocity = FVector::ZeroVector;
};
