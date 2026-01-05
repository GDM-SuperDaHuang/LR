// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/LrDashMovementMode.h"

void ULrDashMovementMode::OnGenerateMove(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	// Super::OnGenerateMove(StartState, TimeStep, OutProposedMove);
	const FVector Dir = StartState.InputCmd.GetLastAffirmativeMoveInput();
	OutProposedMove.LinearVelocity = Dir * DashSpeed;
}

/**
 * Walk 是纯输入驱动,不需要 SyncState,所以 OnSimulationTick 可以是空的 
*/
void ULrDashMovementMode::OnSimulationTick(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	const FDashState* StartState = Params.StartState.SyncState.GetDataByType<FDashState>();
	FDashState& OutState = Output.SyncState.ModifyOrAdd<FDashState>();
	if (!StartState)
	{
		OutState.RemainingTime = DashDuration;
	}
	else
	{
		OutState = *StartState;
	}
	OutState.RemainingTime -= Params.TimeStep.StepMs * 0.001f;
	if (OutState.RemainingTime <= 0.f)
	{
		Output.MovementEndState.NextModeName = TEXT("Walk");
	}
}