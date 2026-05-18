// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "LrAirMovementMode.generated.h"

class ULrMoverComponent;
/**
 * 处理角色自由落体
 */
UCLASS()
class LR_API ULrAirMovementMode : public UBaseMovementMode
{
	GENERATED_BODY()
public:
	ULrAirMovementMode();
	// virtual void Activate() override;
	virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;
	UPROPERTY()
	TObjectPtr<ULrMoverComponent> CacheMoverComponent;
};
