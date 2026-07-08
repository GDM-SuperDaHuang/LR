// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Empty/LrEmptyMovementMode.h"

#include "MoveLibrary/FloorQueryUtils.h"
#include "Mover/LrAllModes.h"

void ULrEmptyMovementMode::Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState)
{
	Super::Activate(Context, PrevModeName, SimContext, StartState, OutSyncState, OutAuxState);
	IsFinish = false;
}

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
	if (IsFinish)
	{
		FFloorCheckResult FloorResult;
		UFloorQueryUtils::FindFloor(
			Params.MovingComps,
			50.f,
			0.f,
			true,
			Params.MovingComps.UpdatedComponent->GetComponentLocation(), FloorResult);

		if (FloorResult.bWalkableFloor)
		{
			OutputState.MovementEndState.NextModeName = LrAllModes::Walk;
		}
		else
		{
			OutputState.MovementEndState.NextModeName = LrAllModes::Air;
		}
	}
}
