// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Walk/LrWalkMovementMode.h"

#include "Kismet/KismetSystemLibrary.h"
#include "MoveLibrary/FloorQueryUtils.h"
#include "MoveLibrary/MovementUtils.h"
#include "Mover/FLrMoverInputCmd.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMovementSettings.h"

ULrWalkMovementMode::ULrWalkMovementMode()
{
	// CacheMoverComponent = Cast<ULrMoverComponent>(GetMoverComponent());//ULrMoverComponent::CDO
}

void ULrWalkMovementMode::Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState)
{
	Super::Activate(Context, PrevModeName, SimContext, StartState, OutSyncState, OutAuxState);
	if (!CacheMoverComponent)
	{
		if (ULrMoverComponent* Mover = Cast<ULrMoverComponent>(GetMoverComponent()))
		{
			CacheMoverComponent = Mover;
		}
	}
}

void ULrWalkMovementMode::GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	Super::GenerateMove_Implementation(SimContext, StartState, TimeStep, OutProposedMove);
	const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	const FLrMoverInputCmd* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();

	if (!SyncState)
	{
		return;
	}

	const float DeltaSeconds = TimeStep.StepMs * 0.001f;
	FVector Velocity = SyncState->GetVelocity_WorldSpace();

	// 保存垂直速度，水平速度将在下方重新计算
	const float SavedZ = Velocity.Z;
	Velocity.Z = 0.f;

	//----------------------------------------------------
	// Input 处理
	//----------------------------------------------------
	FVector MoveIntent = FVector::ZeroVector;
	float SpeedScale = 0.f;

	if (Inputs)
	{
		const FVector RawInput = Inputs->GetMoveInput();
		const float DeadZone = 0.2f;
		const float Magnitude = RawInput.Size();

		if (Magnitude > DeadZone)
		{
			MoveIntent = RawInput.GetSafeNormal();
			SpeedScale = FMath::Clamp((Magnitude - DeadZone) / (1.f - DeadZone), 0.f, 1.f);
		}
	}

	//----------------------------------------------------
	// Movement Config
	//----------------------------------------------------
	const float MaxSpeed = 600.f;
	const float Acceleration = 2500.f;
	const float BrakingDeceleration = 4000.f;
	const float StopThreshold = 5.f;

	//----------------------------------------------------
	// Horizontal Velocity 计算
	//----------------------------------------------------
	if (!MoveIntent.IsNearlyZero())
	{
		const FVector DesiredVelocity = MoveIntent * (MaxSpeed * SpeedScale);
		Velocity = FMath::VInterpConstantTo(Velocity, DesiredVelocity, DeltaSeconds, Acceleration);
	}
	else
	{
		Velocity = FMath::VInterpConstantTo(Velocity, FVector::ZeroVector, DeltaSeconds, BrakingDeceleration);
		if (Velocity.SizeSquared() < FMath::Square(StopThreshold))
		{
			Velocity = FVector::ZeroVector;
		}
	}

	//----------------------------------------------------
	// Jump 处理
	//----------------------------------------------------
	if (Inputs && Inputs->bIsJumpPressed)
	{
		Velocity.Z = CacheMoverComponent->RealisticSettings->JumpImpulseForce;
	}
	else
	{
		Velocity.Z = SavedZ;
	}

	//----------------------------------------------------
	// Output
	//----------------------------------------------------
	OutProposedMove.LinearVelocity = Velocity;
	OutProposedMove.DirectionIntent = MoveIntent;
}



void ULrWalkMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
	const FMoverDefaultSyncState* StartingSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();

	if (!StartingSyncState)
	{
		return;
	}

	TWeakObjectPtr<USceneComponent> UpdatedComp = Params.MovingComps.UpdatedComponent;
	if (!UpdatedComp.Get())
	{
		return;
	}

	const float DeltaTime = Params.TimeStep.StepMs * 0.001f;

	// 提取意图速度
	FVector ProposedVelocity = Params.ProposedMove.LinearVelocity;
	const FVector MoveIntent = Params.ProposedMove.DirectionIntent;

	// 记录初始位置，用于后续计算真实的最终速度 (解决客户端拉回的关键)
	const FVector StartingLocation = UpdatedComp->GetComponentLocation();

	//--------------------------------------------------
	// Rotation 计算
	//--------------------------------------------------
	const FQuat CurrentRotation = StartingSyncState->GetOrientation_WorldSpace().Quaternion();
	FQuat FinalRotation = CurrentRotation;
	const ULrMoverComponent* Mover = Cast<ULrMoverComponent>(GetMoverComponent());

	bool bHasExternalLock = false;
	FRotator ExternalRot = FRotator::ZeroRotator;

	if (Mover)
	{
		bHasExternalLock = Mover->bIsInAttackWarp;
		ExternalRot = Mover->AttackWarpRotation;
	}

	if (bHasExternalLock)
	{
		FinalRotation = ExternalRot.Quaternion();
	}
	else if (!MoveIntent.IsNearlyZero())
	{
		FRotator Desired = MoveIntent.Rotation();
		Desired.Pitch = 0.f;
		Desired.Roll = 0.f;

		constexpr float TurnSpeed = 12.f;
		FRotator Smoothed = FMath::RInterpTo(CurrentRotation.Rotator(), Desired, DeltaTime, TurnSpeed);
		FinalRotation = Smoothed.Quaternion();
	}

	//--------------------------------------------------
	// Move & Slide Along Surface (包含防穿模逻辑)
	//--------------------------------------------------
	const FVector DeltaMove = ProposedVelocity * DeltaTime;
	FHitResult Hit;

	// 尝试进行初步移动
	UpdatedComp->MoveComponent(DeltaMove, FinalRotation, true, &Hit);

	if (Hit.IsValidBlockingHit())
	{
		// 如果撞墙了，计算沿着墙壁滑动的剩余向量
		const float TimeTick = 1.f - Hit.Time;
		if (TimeTick > 0.f)
		{
			FVector SlideDelta = FVector::VectorPlaneProject(DeltaMove * TimeTick, Hit.Normal);
			FHitResult SlideHit;

			// 进行滑动移动
			UpdatedComp->MoveComponent(SlideDelta, FinalRotation, true, &SlideHit);

			// 如果在角落遇到第二次碰撞，进行二次滑动
			if (SlideHit.IsValidBlockingHit())
			{
				const float SecondTimeTick = 1.f - SlideHit.Time;
				if (SecondTimeTick > 0.f)
				{
					FVector SecondSlideDelta = FVector::VectorPlaneProject(SlideDelta * SecondTimeTick, SlideHit.Normal);
					UpdatedComp->MoveComponent(SecondSlideDelta, FinalRotation, true, nullptr);
				}
			}
		}
	}

	//--------------------------------------------------
	// Floor Check
	//--------------------------------------------------
	double IsJump = ProposedVelocity.Z;
	FFloorCheckResult Floor;
	UFloorQueryUtils::FindFloor(
		Params.MovingComps,
		50.f, // Line trace/sweep distance
		0.f,
		true,
		UpdatedComp->GetComponentLocation(),
		Floor);

	//--------------------------------------------------
	// Snap To Floor (解决掉落地下的关键)
	//--------------------------------------------------
	if (Floor.bWalkableFloor && IsJump <= 5.f)
	{
		// 只有当角色悬浮在地面以上（但在检测范围内）时，才向下吸附
		if (Floor.FloorDist > 0.1f && Floor.FloorDist < 10.f)
		{
			FVector DownwardSnap = FVector::DownVector * Floor.FloorDist;

			// 务必使用 Sweep (true) 向下移动，绝对不能用 false 直接强设 Offset
			UpdatedComp->MoveComponent(DownwardSnap, FinalRotation, true, nullptr);
		}
	}
	else
	{
		// 没检测到地面，切换为滞空状态
		OutputState.MovementEndState.NextModeName = LrAllModes::Air;
	}

	//--------------------------------------------------
	// Recalculate Actual Velocity (解决客户端拉回的唯一方法)
	//--------------------------------------------------
	const FVector FinalLocation = UpdatedComp->GetComponentLocation();

	// 通过位移差计算真实的物理速度，而不是使用 ProposedVelocity
	FVector ActualVelocity = (FinalLocation - StartingLocation) / DeltaTime;

	// 如果我们在地面上，强制将 Z 轴速度清零，避免同步状态里的微小抖动
	if (Floor.bWalkableFloor && IsJump <= 5.f)
	{
		ActualVelocity.Z = 0.f;
	}

	//--------------------------------------------------
	// Output Sync State
	//--------------------------------------------------
	OutputSyncState.SetTransforms_WorldSpace(
		FinalLocation,
		FinalRotation.Rotator(),
		ActualVelocity, // 将真实速度汇报给服务器
		FVector::ZeroVector);
}
