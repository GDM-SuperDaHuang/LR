// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Death/LrDeathMovementMode.h"

#include "Mover/LrMoverComponent.h"

void ULrDeathMovementMode::Activate()
{
	Super::Activate();
	bHasEnteredDeath = true;
	ULrMoverComponent* Mover = Cast<ULrMoverComponent>(GetMoverComponent());
	if (!Mover) return;

	// 停掉所有输入影响（关键）
	// Mover->ClearInput();
}

void ULrDeathMovementMode::Deactivate()
{
	Super::Deactivate();
}

void ULrDeathMovementMode::GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	Super::GenerateMove_Implementation(StartState, TimeStep, OutProposedMove);
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
