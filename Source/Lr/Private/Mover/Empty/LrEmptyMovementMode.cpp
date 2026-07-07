// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Empty/LrEmptyMovementMode.h"

void ULrEmptyMovementMode::GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	Super::GenerateMove_Implementation(SimContext, StartState, TimeStep, OutProposedMove);
	// 清除玩家移动输入
	OutProposedMove.DirectionIntent = FVector::ZeroVector;
	OutProposedMove.bHasDirIntent = false;
}

void ULrEmptyMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	Super::SimulationTick_Implementation(Params, OutputState);
}
