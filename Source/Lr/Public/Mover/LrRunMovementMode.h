// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mover/LrBaseMovementMode.h"
#include "LrRunMovementMode.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrRunMovementMode : public ULrBaseMovementMode
{
	GENERATED_BODY()
public:
	static constexpr float MoveSpeed = 600.f;
	static constexpr float Gravity   = -980.f;
	virtual void OnGenerateMove(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void OnSimulationTick(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;

};
