// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Walk/LrWalkMovementMode.h"
#include "Mover/LrMoverComponent.h"

/**
 * 不改位置，
 * 不改 Velocity，只输出「本帧希望往哪加速」
 * @param StartState 
 * @param TimeStep 
 * @param OutProposedMove 
 */
void ULrWalkMovementMode::GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	// 1️⃣ 拿到上一帧的同步状态（位置 / 速度 / 朝向）
	const FLrMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FLrMoverDefaultSyncState>();
	// if (!SyncState)
	// {
	// 	const USceneComponent* UpdatedComp = GetMoverComponent()->GetUpdatedComponent();
	// 	if (UpdatedComp)
	// 	{
	// 		FLrMoverDefaultSyncState& SyncState1 = SyncState;
	// 		FVector Location = UpdatedComp->GetComponentLocation();
	// 		FRotator ComponentRotation = UpdatedComp->GetComponentRotation();
	// 		FVector WorldVelocity = FVector(0.f, 0.f, 0.f);
	// 		SyncState1.SetTransforms_WorldSpace(Location,ComponentRotation,WorldVelocity,WorldVelocity);
	// 	}
	// }
	// 2️⃣ 拿到这一帧的输入
	const FLrDefaultInputs* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FLrDefaultInputs>();

	if (!SyncState)
	{
		OutProposedMove.LinearVelocity = FVector::ZeroVector;
		OutProposedMove.DirectionIntent = FVector::ZeroVector;
		OutProposedMove.AngularVelocityDegrees = FVector::ZeroVector;
	}
	else
	{
		if (!Inputs)
		{
			// 没有输入（可能是 SimProxy），保持原有速度
			OutProposedMove.LinearVelocity = SyncState->GetVelocity_WorldSpace();
			return;
		}

		// 3️⃣ 解析输入
		FVector DesiredMove = Inputs->GetMoveInput_WorldSpace();

		// 行走 = 永远只在水平面
		const FVector Up = GetMoverComponent()->GetUpDirection();
		DesiredMove -= FVector::DotProduct(DesiredMove, Up) * Up;

		// 4️⃣ 根据输入类型决定速度语义
		switch (Inputs->GetMoveInputType())
		{
		case ELrMoveInputType::DirectionalIntent:
			{
				// [-1,1] 意图 → 实际速度
				OutProposedMove.DirectionIntent = DesiredMove;
				DesiredMove = DesiredMove.GetClampedToMaxSize(1.f) * MaxWalkSpeed;
				OutProposedMove.bHasDirIntent = !DesiredMove.IsNearlyZero();
			}
			break;

		case ELrMoveInputType::Velocity:
			{
				// 直接给速度，但限制上限
				DesiredMove = DesiredMove.GetClampedToMaxSize(MaxWalkSpeed);
				OutProposedMove.DirectionIntent =
					MaxWalkSpeed > KINDA_SMALL_NUMBER
						? DesiredMove / MaxWalkSpeed
						: FVector::ZeroVector;

				OutProposedMove.bHasDirIntent = !DesiredMove.IsNearlyZero();
			}
			break;

		default:
			{
				DesiredMove = FVector::ZeroVector;
				OutProposedMove.DirectionIntent = FVector::ZeroVector;
				OutProposedMove.bHasDirIntent = false;
			}
			break;
		}

		// 5️⃣ 写入“期望速度”
		OutProposedMove.LinearVelocity = DesiredMove;

		// 6️⃣ 面朝方向（可选）
		FVector FacingDir = Inputs->GetOrientationIntentDir_WorldSpace();
		FacingDir -= FVector::DotProduct(FacingDir, Up) * Up;

		if (FacingDir.Normalize())
		{
			const FQuat Current = SyncState->GetOrientation_WorldSpace().Quaternion();
			const FQuat Desired = FQuat::FindBetweenNormals(FVector::ForwardVector, FacingDir);
			OutProposedMove.AngularVelocityDegrees = (Current.Inverse() * Desired).ToRotationVector() / TimeStep.StepMs * 1000.f;
		}
	}
}

void ULrWalkMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	const float DeltaSeconds = Params.TimeStep.StepMs * 0.001f;

	// 1️⃣ 读取起始同步状态
	const FLrMoverDefaultSyncState* StartSync = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FLrMoverDefaultSyncState>();
	if (!StartSync)
	{
		return;
	}

	// 2️⃣ 读取 GenerateMove 产生的结果
	const FProposedMove& Move = Params.ProposedMove;

	// 3️⃣ 速度积分 → 位置
	FVector NewVelocity = Move.LinearVelocity;
	FVector NewLocation = StartSync->GetLocation_WorldSpace() + NewVelocity * DeltaSeconds;

	// 4️⃣ 朝向积分
	FRotator NewOrientation = StartSync->GetOrientation_WorldSpace() + FRotator::MakeFromEuler(Move.AngularVelocityDegrees * DeltaSeconds);

	// 5️⃣ 写回新的 SyncState
	FLrMoverDefaultSyncState& OutSync = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FLrMoverDefaultSyncState>();

	OutSync.SetTransforms_WorldSpace(
		NewLocation,
		NewOrientation,
		NewVelocity,
		Move.AngularVelocityDegrees
	);

	// 6️⃣ 保存 intent（给动画 / 预测用）
	OutSync.MoveDirectionIntent = Move.DirectionIntent;
}
