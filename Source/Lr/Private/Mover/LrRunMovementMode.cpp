// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/LrRunMovementMode.h"

void ULrRunMovementMode::OnGenerateMove(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	// Super::OnGenerateMove(StartState, TimeStep, OutProposedMove);
	const FVector Input = StartState.InputCmd.GetMoveIntent();
	OutProposedMove.LinearVelocity = Input * MoveSpeed;
}

/**
 * Walk 是纯输入驱动,不需要 SyncState,所以 OnSimulationTick 可以是空的 
*/
void ULrRunMovementMode::OnSimulationTick(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	// Super::OnSimulationTick(Params, OutputState);
	// Walk 没有额外状态
}

