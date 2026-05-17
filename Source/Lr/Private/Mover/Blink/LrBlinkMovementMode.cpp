// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Blink/LrBlinkMovementMode.h"

#include "MoveLibrary/FloorQueryUtils.h"
#include "Mover/FLrMoverInputCmd.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/LrAllModes.h"

void ULrBlinkMovementMode::Activate()
{
	Super::Activate();
	ElapsedTime = 0.f;

	ULrMoverComponent* Mover = Cast<ULrMoverComponent>(GetMoverComponent());

	if (!Mover) return;

	const FMoverSyncState& SyncState = Mover->GetSyncState();
	const FMoverDefaultSyncState* DefaultSync = SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();

	if (!DefaultSync) return;

	// 默认朝向
	BlinkDirection = DefaultSync->GetOrientation_WorldSpace()
	                            .Vector()
	                            .GetSafeNormal2D();
	// 尝试读取输入方向
	const FMoverInputCmdContext& InputCmd = Mover->GetLastInputCmd();
	const FLrMoverInputCmd* Inputs = InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();

	if (Inputs)
	{
		FVector MoveInput = Inputs->GetMoveInput();
		MoveInput.Z = 0.f;
		if (!MoveInput.IsNearlyZero())
		{
			BlinkDirection = MoveInput.GetSafeNormal();
		}
	}
	BlinkDirection.Z = 0.f;
	BlinkDirection.Normalize();
}

void ULrBlinkMovementMode::Deactivate()
{
	Super::Deactivate();
}

void ULrBlinkMovementMode::GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	float BlinkSpeed = BlinkDistance / BlinkDuration;

	FVector Velocity = BlinkDirection * BlinkSpeed;

	// 保留重力
	if (!bIgnoreGravity)
	{
		const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();

		if (SyncState)
		{
			Velocity.Z = SyncState->GetVelocity_WorldSpace().Z;
		}
	}

	OutProposedMove.LinearVelocity = Velocity;

	OutProposedMove.DirectionIntent = BlinkDirection;
}

void ULrBlinkMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
	const FMoverDefaultSyncState* StartSync = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	check(StartSync);
	float DeltaTime = Params.TimeStep.StepMs * 0.001f;
	ElapsedTime += DeltaTime;

	FQuat Rotation = StartSync->GetOrientation_WorldSpace().Quaternion();

	FVector Delta = Params.ProposedMove.LinearVelocity * DeltaTime;

	FHitResult Hit;

	Params.MovingComps.UpdatedComponent->MoveComponent(
		Delta,
		Rotation,
		true,
		&Hit);

	// 撞墙结束
	if (Hit.IsValidBlockingHit())
	{
		ElapsedTime = BlinkDuration;
	}

	// 闪现结束
	if (ElapsedTime >= BlinkDuration)
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

	OutputSyncState.SetTransforms_WorldSpace(
		Params.MovingComps.UpdatedComponent->GetComponentLocation(),
		Rotation.Rotator(),
		Params.ProposedMove.LinearVelocity,
		FVector::ZeroVector);
}
