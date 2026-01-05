// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/LrJumpMovementMode.h"

void ULrJumpMovementMode::OnGenerateMove(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	// Super::OnGenerateMove(StartState, TimeStep, OutProposedMove);
	OutProposedMove.LinearVelocity = FVector::UpVector * JumpVelocity;

}

/**
 * Walk 是纯输入驱动,不需要 SyncState,所以 OnSimulationTick 可以是空的 
*/
void ULrJumpMovementMode::OnSimulationTick(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	Output.MovementEndState.NextModeName = TEXT("Falling");
}