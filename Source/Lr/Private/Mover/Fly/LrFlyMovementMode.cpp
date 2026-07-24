// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Fly/LrFlyMovementMode.h"

#include "Mover/FLrMoverInputCmd.h"

ULrFlyMovementMode::ULrFlyMovementMode()
{
}

void ULrFlyMovementMode::Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState)
{
	// 调用父类激活逻辑，保持 Mover 系统的标准生命周期
	Super::Activate(Context, PrevModeName, SimContext, StartState, OutSyncState, OutAuxState);
}

void ULrFlyMovementMode::GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	Super::GenerateMove_Implementation(SimContext, StartState, TimeStep, OutProposedMove);

	// 获取当前输入命令（包含移动方向、按键状态等）
	const FLrMoverInputCmd* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();
	if (!Inputs) return;

	// 将输入方向直接作为飞行方向，并按固定速度缩放
	FVector Input = Inputs->GetMoveInput();
	float Speed = 1200.f;
	FVector DesiredVelocity = Input.GetSafeNormal() * Speed;

	OutProposedMove.LinearVelocity = DesiredVelocity;
	OutProposedMove.DirectionIntent = Input;
}

void ULrFlyMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	Super::SimulationTick_Implementation(Params, OutputState);
	FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
	const float DeltaTime = Params.TimeStep.StepMs * 0.001f;
	TWeakObjectPtr<USceneComponent> UpdatedComp = Params.MovingComps.UpdatedComponent;
	if (!UpdatedComp.Get())
	{
		return;
	}

	//------------------------------------
	// 计算旋转
	//------------------------------------
	FQuat CurrentRotation = UpdatedComp->GetComponentQuat();
	FQuat FinalRotation = CurrentRotation;
	FVector MoveIntent = Params.ProposedMove.DirectionIntent;
	if (!MoveIntent.IsNearlyZero())
	{
		// 飞行朝向
		FRotator DesiredRotation = MoveIntent.Rotation();
		// 如果不希望飞机式翻滚
		// 锁 Pitch/Roll
		DesiredRotation.Roll = 0.f;
		const float TurnSpeed = 8.f;
		FinalRotation = FMath::QInterpTo(
			CurrentRotation,
			DesiredRotation.Quaternion(),
			DeltaTime,
			TurnSpeed
		);
	}


	//------------------------------------
	// 移动
	//------------------------------------
	FVector MoveDelta = Params.ProposedMove.LinearVelocity * DeltaTime;
	FHitResult Hit;
	UpdatedComp->MoveComponent(
		MoveDelta,
		FinalRotation,
		true,
		&Hit
	);


	//------------------------------------
	// 写回
	//------------------------------------
	const FVector Location = UpdatedComp->GetComponentLocation();
	OutputSyncState.SetTransforms_WorldSpace(
		Location,
		FinalRotation.Rotator(),
		Params.ProposedMove.LinearVelocity,
		FVector::ZeroVector
	);
}
