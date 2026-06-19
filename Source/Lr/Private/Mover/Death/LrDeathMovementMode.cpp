// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Death/LrDeathMovementMode.h"

#include "Mover/LrMoverComponent.h"

void ULrDeathMovementMode::Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState)
{
	Super::Activate(Context, PrevModeName, SimContext, StartState, OutSyncState, OutAuxState);
	bHasEnteredDeath = true;
	ULrMoverComponent* Mover = Cast<ULrMoverComponent>(GetMoverComponent());
	if (!Mover) return;
}

void ULrDeathMovementMode::GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	Super::GenerateMove_Implementation(SimContext, StartState, TimeStep, OutProposedMove);
	// ❗死亡状态不再产生移动
	OutProposedMove.LinearVelocity = FVector::ZeroVector;
	OutProposedMove.AngularVelocityDegrees = FVector::ZeroVector;
}

void ULrDeathMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	FMoverDefaultSyncState& Sync = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
	const FVector CurrentLocation = Params.MovingComps.UpdatedComponent->GetComponentLocation();
	const FQuat CurrentRotation = Sync.GetOrientation_WorldSpace().Quaternion();
	// ❗保持静止
	OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>().SetTransforms_WorldSpace(
		CurrentLocation,
		CurrentRotation.Rotator(),
		FVector::ZeroVector,
		FVector::ZeroVector);
}
