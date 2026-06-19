// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "LrKnockbackMovementMode.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrKnockbackMovementMode : public UBaseMovementMode
{
	GENERATED_BODY()
public:

	virtual void Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState) override;
	virtual void GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params,FMoverTickEndData& OutputState) override;

protected:
	// 初速度（击飞核心）
	FVector InitialVelocity = FVector::ZeroVector;
	// 重力
	float Gravity = 980.f;
	// 持续时间
	float Duration = 0.5f;

private:
	float ElapsedTime = 0.f;
};
