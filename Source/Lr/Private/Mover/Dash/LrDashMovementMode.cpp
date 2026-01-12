// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/LrDashMovementMode.h"

/**
 * Network Prediction 可能在一帧内：调用多次
 */
void ULrDashMovementMode::OnGenerateMove(const FMoverTickStartData& Start	State, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{

	/** 
	 * Dash 和 Zipline 一样：
	 * 不吃输入
	 * 不做预测位移
	 * 只在 SimulationTick 中移动
	*/
	OutProposedMove = FProposedMove();
}

/**
 * Walk 是纯输入驱动,不需要 SyncState,所以 OnSimulationTick 可以是空的 
*/
void ULrDashMovementMode::OnSimulationTick(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{

	const float DeltaSeconds = Params.TimeStep.StepSeconds;

	const FDashState* StartDashState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FDashState>();

	FDashState DashState;

	// -------- 初始化 Dash --------
	if (!StartDashState)
	{
		DashState.ElapsedTime = 0.f;
		DashState.Duration = DashDuration;

		// 方向来源：最后一次有效移动输入
		FVector DashDir = Params.StartState.SyncState.GetVelocity_WorldSpace();
		if (DashDir.IsNearlyZero())
		{
			DashDir = Params.StartState.SyncState.GetTransform().GetRotation().Vector();
		}

		DashState.DashDirection = DashDir.GetSafeNormal();
	}
	else
	{
		DashState = *StartDashState;
	}

	// -------- 时间推进 --------
	const float RemainingTime = DashState.Duration - DashState.ElapsedTime;
	const float MoveTime = FMath::Min(RemainingTime, DeltaSeconds);

	const FVector MoveDelta = DashState.DashDirection * DashSpeed * MoveTime;

	// -------- 实际移动 --------
	FHitResult Hit;
	UMovementUtils::TrySafeMoveUpdatedComponent(
		Params.UpdatedComponent,
		MoveDelta,
		Params.StartState.SyncState.GetRotation(),
		true,
		Hit
	);

	// -------- 写 SyncState --------
	const FVector NewLocation = Params.UpdatedComponent->GetComponentLocation();
	const FVector Velocity = DashState.DashDirection * DashSpeed;

	OutputState.SyncState.SyncStateCollection.AddOrReplaceData(DashState);

	OutputState.SyncState.SetTransforms_WorldSpace(
		NewLocation,
		Params.StartState.SyncState.GetRotation(),
		Velocity,
		FVector::ZeroVector,
		nullptr
	);

	DashState.ElapsedTime += MoveTime;

	// -------- Dash 结束 --------
	if (DashState.ElapsedTime >= DashState.Duration)
	{
		OutputState.MovementEndState.NextModeName = DefaultModeNames::Walking;
		OutputState.MovementEndState.RemainingMs = Params.TimeStep.StepMs - FMath::RoundToInt(MoveTime * 1000.f);
	}
}