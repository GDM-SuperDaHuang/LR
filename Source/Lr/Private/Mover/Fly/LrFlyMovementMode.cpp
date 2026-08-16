// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Fly/LrFlyMovementMode.h"

#include "Mover/FLrMoverInputCmd.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMoverComponent.h"

ULrFlyMovementMode::ULrFlyMovementMode()
{
}

void ULrFlyMovementMode::Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState)
{
	// 调用父类激活逻辑，保持 Mover 系统的标准生命周期
	Super::Activate(Context, PrevModeName, SimContext, StartState, OutSyncState, OutAuxState);

	// 缓存所属的 Mover 组件，避免后续每帧重复查找
	if (!CacheMoverComponent)
	{
		CacheMoverComponent = Cast<ULrMoverComponent>(GetMoverComponent());
	}
}

void ULrFlyMovementMode::GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
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

	//----------------------------------------------------
	// Input 处理（含相机相对方向转换）
	// 水平分量只使用相机 Yaw（俯视角相机 Pitch 固定，直接使用会向地面俯冲）
	// 垂直分量使用输入的 Z 轴（输入配置 Vertical 轴 X->Z），映射为世界 Z
	//----------------------------------------------------
	FVector MoveIntent = FVector::ZeroVector;
	float SpeedScale = 0.f;
	float VerticalIntent = 0.f;

	if (Inputs)
	{
		const FVector RawInput = Inputs->GetMoveInput();
		constexpr float DeadZone = 0.2f;

		// 水平分量
		const FVector HorizontalInput(RawInput.X, RawInput.Y, 0.f);
		const float HorizontalMag = HorizontalInput.Size();
		if (HorizontalMag > DeadZone)
		{

			const FRotator CamRot = Inputs->ControlRotation;
			const FVector ForwardDir = FRotationMatrix(CamRot).GetUnitAxis(EAxis::X);
			const FVector RightDir   = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Y);
			const FVector UpDir      = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Z);
			
			// 假设 RawInput.X 是左右，RawInput.Y 是前后，RawInput.Z 是上下
			FVector WorldDirection = ForwardDir * RawInput.Y + RightDir * RawInput.X + UpDir * RawInput.Z;
			MoveIntent = WorldDirection.GetSafeNormal();

			
			// const FRotator YawRot(0.f, Inputs->ControlRotation.Yaw, 0.f);
			// const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
			// const FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
			// const FVector WorldHorizontal = ForwardDir * RawInput.Y + RightDir * RawInput.X;
			// MoveIntent = WorldHorizontal.GetSafeNormal();
			
			SpeedScale = FMath::Clamp((HorizontalMag - DeadZone) / (1.f - DeadZone), 0.f, 1.f);
		}

		// 垂直分量（上升/下降）
		if (FMath::Abs(RawInput.Z) > DeadZone)
		{
			const float VerticalScale = FMath::Clamp((FMath::Abs(RawInput.Z) - DeadZone) / (1.f - DeadZone), 0.f, 1.f);
			VerticalIntent = VerticalScale * FMath::Sign(RawInput.Z);
		}
	}

	//----------------------------------------------------
	// 期望速度 = 水平意图 + 垂直意图
	//----------------------------------------------------
	const FVector DesiredVelocity = MoveIntent * (MaxSpeed * SpeedScale) + FVector(0.f, 0.f, MaxVerticalSpeed * VerticalIntent);

	if (!DesiredVelocity.IsNearlyZero())
	{
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
	// Output
	//----------------------------------------------------
	OutProposedMove.LinearVelocity = Velocity;
	OutProposedMove.DirectionIntent = MoveIntent;
}

void ULrFlyMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	Super::SimulationTick_Implementation(Params, OutputState);
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
	if (DeltaTime <= 0.f)
	{
		return;
	}

	const FVector ProposedVelocity = Params.ProposedMove.LinearVelocity;
	const FVector MoveIntent = Params.ProposedMove.DirectionIntent;
	const FVector StartingLocation = UpdatedComp->GetComponentLocation();

	//--------------------------------------------------
	// 跳跃退出飞行模式 -> 自由落体
	//--------------------------------------------------
	// const FLrMoverInputCmd* Inputs = Params.StartState.InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();
	// if (Inputs && Inputs->bIsJumpPressed)
	// {
	// 	OutputState.MovementEndState.NextModeName = LrAllModes::Air;
	// }

	//--------------------------------------------------
	// Rotation 计算（像飞机倾斜，缓慢过渡）
	//--------------------------------------------------
	const FQuat CurrentRotation = StartingSyncState->GetOrientation_WorldSpace().Quaternion();
	FQuat FinalRotation = CurrentRotation; // 默认保持当前旋转

	const FLrMoverInputCmd* Inputs = Params.StartState.InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();
	if (Inputs)
	{
		const FVector RawInput = Inputs->GetMoveInput();
		constexpr float DeadZone = 0.2f;

		// 检查是否有显著水平或垂直移动输入
		const FVector HorizontalInput(RawInput.X, RawInput.Y, 0.f);
		const float HorizontalMag = HorizontalInput.Size();
		const bool bHasMoveInput = (HorizontalMag > DeadZone) || (FMath::Abs(RawInput.Z) > DeadZone);

		if (bHasMoveInput)
		{
			// 有移动输入时，根据移动方向计算倾斜角度（像飞机）
			// Roll: 左右输入控制 (-60 to 60 degrees)
			const float TargetRoll = FMath::Clamp(RawInput.X * RollAngle, -RollAngle, RollAngle);
			
			// Pitch: 上下输入控制抬头低头 (-80 to 80 degrees)
			const float TargetPitch = FMath::Clamp(RawInput.Z * PitchAngle, -PitchAngle, PitchAngle);
			
			// 使用 FInterpTo 实现平滑过渡 - 使用成员变量确保每个角色独立
			RollInterpValue = FMath::FInterpTo(RollInterpValue, TargetRoll, DeltaTime, TurnSpeed * 3.f);
			const float SmoothedRoll = FMath::Clamp(RollInterpValue, -RollAngle, RollAngle);
			
			// Pitch interpolation using same mechanism
			PitchInterpValue = FMath::FInterpTo(PitchInterpValue, TargetPitch, DeltaTime, TurnSpeed * 3.f);
			const float SmoothedPitch = FMath::Clamp(PitchInterpValue, -PitchAngle, PitchAngle);

			
			// 保持 Yaw 为相机方向，修改 Roll 和 Pitch
			FRotator DesiredRot = Inputs->ControlRotation;
			DesiredRot.Pitch = SmoothedPitch; // 使用插值后的Pitch
			DesiredRot.Roll = SmoothedRoll;
			// 从 FRotator 转换为 FQuat
			FinalRotation = DesiredRot.Quaternion();
		}
		else
		{
			// 无移动输入时，回正 Roll 和 Pitch 到 0（像飞机回正）
			// 使用 FInterpTo 实现平滑回正
			RollInterpValue = FMath::FInterpTo(RollInterpValue, 0.f, DeltaTime, TurnSpeed * 5.f);
			const float SmoothedRoll = FMath::Clamp(RollInterpValue, -RollAngle, RollAngle);
			
			// Pitch also interpolates to 0
			PitchInterpValue = FMath::FInterpTo(PitchInterpValue, 0.f, DeltaTime, TurnSpeed * 5.f);
			const float SmoothedPitch = FMath::Clamp(PitchInterpValue, -PitchAngle, PitchAngle);

			// 创建目标旋转：Yaw 保持相机方向，Roll 和 Pitch 插值到 0
			FRotator TargetRot = Inputs->ControlRotation;
			TargetRot.Pitch = SmoothedPitch;
			TargetRot.Roll = SmoothedRoll;
			// 从 FRotator 转换为 FQuat
			FinalRotation = TargetRot.Quaternion();
		}
	}

	//--------------------------------------------------
	// 迭代式移动与滑动（防卡墙、穿模），与 Walk 模式一致
	//--------------------------------------------------
	FVector RemainingMove = ProposedVelocity * DeltaTime;
	bool bHitWalkableFloor = false;
	constexpr int32 MaxSimulations = 4;

	for (int32 i = 0; i < MaxSimulations && !RemainingMove.IsNearlyZero(); ++i)
	{
		FHitResult Hit;
		UpdatedComp->MoveComponent(RemainingMove, FinalRotation, true, &Hit);

		// 处理初始穿模：向外推出，解除穿模（不消耗本帧移动力）
		if (Hit.bStartPenetrating)
		{
			const FVector DepenetrationVector = Hit.Normal * (Hit.PenetrationDepth + 0.1f);
			UpdatedComp->MoveComponent(DepenetrationVector, FinalRotation, true, nullptr);
			continue;
		}

		if (Hit.IsValidBlockingHit())
		{
			// 剩余移动量 = 未消耗的移动量沿碰撞面滑动
			RemainingMove = RemainingMove * (1.f - Hit.Time);
			RemainingMove = FVector::VectorPlaneProject(RemainingMove, Hit.Normal);

			// 向下碰到可站立地面 -> 落地
			if (Hit.Normal.Z >= 0.7f && ProposedVelocity.Z <= 0.f)
			{
				bHitWalkableFloor = true;
				RemainingMove.Z = FMath::Min(RemainingMove.Z, 0.f);
			}
		}
		else
		{
			// 没有碰撞，移动全部完成
			RemainingMove = FVector::ZeroVector;
			break;
		}
	}
	
	//--------------------------------------------------
	// Recalculate Actual Velocity（解决客户端拉回）
	//--------------------------------------------------
	const FVector FinalLocation = UpdatedComp->GetComponentLocation();
	FVector ActualVelocity = (FinalLocation - StartingLocation) / DeltaTime;

	if (bHitWalkableFloor && ProposedVelocity.Z <= 0.f)
	{
		ActualVelocity.Z = 0.f;
	}

	OutputSyncState.SetTransforms_WorldSpace(
		FinalLocation,
		FinalRotation.Rotator(),
		ActualVelocity,
		FVector::ZeroVector);
}
