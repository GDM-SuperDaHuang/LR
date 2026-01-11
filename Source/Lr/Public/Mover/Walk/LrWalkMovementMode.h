// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "LrWalkMovementMode.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrWalkMovementMode : public UBaseMovementMode
{
	GENERATED_BODY()
	
public:
	virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;

protected:
	UPROPERTY(EditAnywhere, Category="Walk")
	float MaxWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category="Walk")
	float Acceleration = 2048.f;

	UPROPERTY(EditAnywhere, Category="Walk")
	float GroundFriction = 8.f;
};


