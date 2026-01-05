// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/LrRunMovementMode.h"

void ULrRunMovementMode::OnGenerateMove(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	Super::OnGenerateMove(StartState, TimeStep, OutProposedMove);

	const FLrMoverInputCmd* Input = static_cast<const FLrMoverInputCmd*>(StartState.InputCmd);

	// 计算移动方向：从输入的 X 和 Y 分量构建向量，Z 为 0（平面移动）。
	// Dir 被限制在单位向量长度内，防止速度超过预期。
	FVector Dir(Input->MoveInput.X, Input->MoveInput.Y, 0.f);
	Dir = Dir.GetClampedToMaxSize(1.f);

	// 设置输出移动的线性速度：方向乘以移动速度，Z 分量保持当前状态（用于垂直速度，如跳跃）。
	// 填充到OnSimulationTick 的 FMoverSyncState& OutState
	OutProposedMove.LinearVelocity = Dir * MoveSpeed;

	// OutProposedMove.LinearVelocity.Z = Params.CurrentState.LinearVelocity.Z;
	//
	// // ===== Mode 决策（GASP 核心）=====
	// //Params.CurrentState	上一帧 的最终 SyncState（只读），由 OnSimulationTick 上一帧的写入
	// //Params.OutputSyncState ：意图,还不是还不是权威状态，可能被，被丢弃 / 回滚，	这一帧 要写入的 SyncState（可写）
	// const FLrMoverSyncState& Sync = static_cast<const FLrMoverSyncState&>(Params.CurrentState);
	//
	// // 这里进行模式切换决策，但不实际修改状态（因为函数可能被回滚）。
	// if (Input->bDashPressed && Sync.DashTimeRemaining <= 0.f)
	// {
	// 	Sync.DashDirection = Dir.IsNearlyZero() ? FVector::ForwardVector : Dir;
	// 	Sync.DashTimeRemaining = 0.25f;
	// 	Params.MoverComponent->QueueNextMode(TEXT("Dash"));
	// }
	//
	// // 检查 Jump 输入：如果 Jump 按下且当前在地面上，则队列下一个模式为 "Jump"。
	// if (Input->bJumpPressed && Sync.bIsOnGround)
	// {
	// 	Params.MoverComponent->QueueNextMode(TEXT("Jump"));
	// }
}


void ULrRunMovementMode::OnSimulationTick(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	Super::OnSimulationTick(Params, OutputState);

	// Cannot cast from FMoverTickEndData to FLrMoverSyncState& via static_cast
	FLrMoverSyncState& Sync = static_cast<FLrMoverSyncState&>(OutputState.SyncState);

	if (!Sync.bIsOnGround)
	{
		
		Sync.LinearVelocity.Z += Gravity * Params.StepMS * 0.001f;
	}

	Sync.Position += Sync.LinearVelocity * Params.StepMS * 0.001f;
}

