// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mover/LrBaseMovementMode.h"
#include "LrJumpMovementMode.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrJumpMovementMode : public ULrBaseMovementMode
{
	GENERATED_BODY()
public:
	virtual void OnGenerateMove(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void OnSimulationTick(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;

};


// UCLASS()
// class UFallingMovementMode : public UBaseMovementMode
// {
// 	GENERATED_BODY()

// public:
// 	UPROPERTY(EditAnywhere)
// 	float Gravity = 980.f;

// 	virtual void OnGenerateMove(
// 		const FMoverTickStartData& Start,
// 		const FMoverTimeStep& Step,
// 		FProposedMove& OutMove
// 	) const override
// 	{
// 		OutMove.LinearVelocity =
// 			Start.SyncState.GetVelocity() -
// 			FVector::UpVector * Gravity * Step.StepMs * 0.001f;
// 	}

// 	virtual void OnSimulationTick(
// 		const FSimulationTickParams& Params,
// 		FMoverTickEndData& Output
// 	) override
// 	{
// 		if (Params.HitResult.bBlockingHit)
// 		{
// 			Output.MovementEndState.NextModeName = TEXT("Walk");
// 		}
// 	}
// };

// Walk → Dash
UCLASS()
class UDashTransition : public UBaseMovementModeTransition
{
	GENERATED_BODY()

	virtual FTransitionEvalResult OnEvaluate(
		const FSimulationTickParams& Params
	) const override
	{
		const auto* Inputs =
			Params.InputCmd.InputCollection.FindDataByType<FSimpleAbilityInputs>();

		if (Inputs && Inputs->bDashJustPressed)
		{
			return FTransitionEvalResult::RequestTransition(TEXT("Dash"));
		}
		return FTransitionEvalResult::NoTransition();
	}
};

// Walk → Jump
UCLASS()
class UJumpTransition : public UBaseMovementModeTransition
{
	GENERATED_BODY()

	virtual FTransitionEvalResult OnEvaluate(
		const FSimulationTickParams& Params
	) const override
	{
		const auto* Inputs =
			Params.InputCmd.InputCollection.FindDataByType<FSimpleAbilityInputs>();

		if (Inputs && Inputs->bJumpJustPressed)
		{
			return FTransitionEvalResult::RequestTransition(TEXT("Jump"));
		}
		return FTransitionEvalResult::NoTransition();
	}
};
