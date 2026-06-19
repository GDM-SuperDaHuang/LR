// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Blink/LrKnockbackMovementMode.h"

#include "MoveLibrary/FloorQueryUtils.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMoverComponent.h"

void ULrKnockbackMovementMode::Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState)
{
	Super::Activate(Context, PrevModeName, SimContext, StartState, OutSyncState, OutAuxState);
	ElapsedTime = 0.f;
	ULrMoverComponent* Mover = Cast<ULrMoverComponent>(GetMoverComponent());
	if (!Mover) return;
	// 从外部输入缓存（关键点）
	if (const FPendingLaunchData* Data = Mover->GetPendingLaunchData())
	{
		InitialVelocity = Data->Impulse;
		Duration = Data->Duration;
	}
}

//提议
void ULrKnockbackMovementMode::GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	Super::GenerateMove_Implementation(SimContext, StartState, TimeStep, OutProposedMove);
	float DeltaTime = TimeStep.StepMs * 0.001f;
	FVector Velocity = InitialVelocity;
	// 重力影响（击飞一定要有）
	Velocity.Z -= Gravity * ElapsedTime;
	OutProposedMove.LinearVelocity = Velocity;
}


//执行
void ULrKnockbackMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	ElapsedTime += Params.TimeStep.StepMs * 0.001f;
	FMoverDefaultSyncState& Sync = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
	const FVector DeltaMove = Params.ProposedMove.LinearVelocity * (Params.TimeStep.StepMs * 0.001f);
	FHitResult Hit;

	Params.MovingComps.UpdatedComponent->MoveComponent(
		DeltaMove,
		Sync.GetOrientation_WorldSpace().Quaternion(),
		true,
		&Hit);

	// 时间结束
	if (ElapsedTime >= Duration)
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
	
	Sync.SetTransforms_WorldSpace(
		Params.MovingComps.UpdatedComponent->GetComponentLocation(),
		Sync.GetOrientation_WorldSpace(),
		Params.ProposedMove.LinearVelocity,
		FVector::ZeroVector);
}
