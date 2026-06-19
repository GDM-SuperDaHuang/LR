// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "LrBlinkMovementMode.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrBlinkMovementMode : public UBaseMovementMode
{
	GENERATED_BODY()
public:
	
	// virtual void Activate() override;
	virtual void Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState) override;
	virtual void GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;

protected:
	UPROPERTY(EditAnywhere)
	float BlinkDistance = 500.f;// 闪现距离

	UPROPERTY(EditAnywhere)
	float BlinkDuration = 0.2f;// 闪现持续时间

	UPROPERTY(EditAnywhere)
	bool bIgnoreGravity = true;// 是否无视重力

	
private:
	// 当前闪现方向
	UPROPERTY()
	mutable FVector BlinkDirection = FVector::ForwardVector;

	// 已经过时间
	UPROPERTY()
	float ElapsedTime = 0.f;

};
