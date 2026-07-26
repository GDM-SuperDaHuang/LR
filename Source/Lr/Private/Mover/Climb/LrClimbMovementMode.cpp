// Fill out your copyright notice in the Description page of Project Settings.

#include "Mover/Climb/LrClimbMovementMode.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "MoverComponent.h"
#include "Mover/FLrMoverInputCmd.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMoverComponent.h"

ULrClimbMovementMode::ULrClimbMovementMode()
{
}

void ULrClimbMovementMode::UpdateWallBasis(const FVector& InNormal)
{
	WallNormal = InNormal.GetSafeNormal();
	WallUp = (FVector::UpVector - FVector::UpVector.ProjectOnToNormal(WallNormal)).GetSafeNormal();
	WallRight = FVector::CrossProduct(WallUp, WallNormal).GetSafeNormal();
}

void ULrClimbMovementMode::Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext,
                                    const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState)
{
	Super::Activate(Context, PrevModeName, SimContext, StartState, OutSyncState, OutAuxState);

	if (!CacheMoverComponent)
	{
		CacheMoverComponent = Cast<ULrMoverComponent>(GetMoverComponent());
	}

	USceneComponent* UpdatedComp = GetMoverComponent()->GetUpdatedComponent();
	if (!UpdatedComp) return;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetMoverComponent()->GetOwner());

	const FVector Start = UpdatedComp->GetComponentLocation();
	const FVector Forward = UpdatedComp->GetForwardVector();

	//--------------------------------------------------
	// 1. 向前射线找墙
	//--------------------------------------------------
	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, Start + Forward * WallSearchDistance, ECC_WorldStatic, QueryParams);

	//--------------------------------------------------
	// 2. 射线失败 → 球体扫描（更鲁棒，处理角色已在墙内的情况）
	//--------------------------------------------------
	if (!bHit)
	{
		const FCollisionShape Sphere = FCollisionShape::MakeSphere(WallSearchDistance * 0.5f);
		bHit = GetWorld()->SweepSingleByChannel(
			Hit, Start, Start + Forward * WallSearchDistance, FQuat::Identity,
			ECC_WorldStatic, Sphere, QueryParams);
	}

	if (!bHit) return;

	// 拒绝过于平坦的表面（与地面类似）
	if (FVector::DotProduct(Hit.Normal, FVector::UpVector) > 0.75f) return;

	// 建立墙面坐标系
	UpdateWallBasis(Hit.Normal);

	//--------------------------------------------------
	// 3. 沿法线吸附到墙面
	//--------------------------------------------------
	const FVector TargetPos = Hit.Location + WallNormal * StickDistance;
	const FVector SnapDelta = TargetPos - Start;
	if (!SnapDelta.IsNearlyZero())
	{
		FHitResult SnapHit;
		UpdatedComp->MoveComponent(SnapDelta, UpdatedComp->GetComponentQuat(), true, &SnapHit);
	}

	//	//--------------------------------------------------
	// 4. 立即设置旋转（修复进入时姿势延迟）
	//--------------------------------------------------
	const FRotator DesiredRotation = FRotationMatrix::MakeFromXY(WallUp, -WallRight).Rotator();
	UpdatedComp->SetWorldRotation(DesiredRotation);
}

void ULrClimbMovementMode::GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState,
                                                       const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	Super::GenerateMove_Implementation(SimContext, StartState, TimeStep, OutProposedMove);

	const FLrMoverInputCmd* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();
	if (!Inputs) return;

	const FVector MoveInput = Inputs->GetMoveInput();

	// 用摄像机朝向分解输入意图，再映射到墙面坐标
	// W/S (forward) → WallUp（上下），A/D (right) → WallRight（左右）
	const FRotator CamRot = Inputs->ControlRotation;
	const FVector CamForward = FRotationMatrix(CamRot).GetScaledAxis(EAxis::X);
	const FVector CamRight = FRotationMatrix(CamRot).GetScaledAxis(EAxis::Y);

	const float ForwardAmount = FVector::DotProduct(MoveInput, CamForward);
	const float RightAmount = FVector::DotProduct(MoveInput, CamRight);

	const FVector Velocity = (WallUp * ForwardAmount - WallRight * RightAmount) * ClimbSpeed;

	OutProposedMove.LinearVelocity = Velocity;
	OutProposedMove.DirectionIntent = MoveInput;
}

void ULrClimbMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
	const FMoverDefaultSyncState* StartingSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	if (!StartingSyncState) return;

	TWeakObjectPtr<USceneComponent> UpdatedComp = Params.MovingComps.UpdatedComponent;
	if (!UpdatedComp.Get()) return;

	const float DeltaTime = Params.TimeStep.StepMs * 0.001f;
	const FVector StartingLocation = UpdatedComp->GetComponentLocation();
	FQuat CurrentRotation = StartingSyncState->GetOrientation_WorldSpace().Quaternion();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetMoverComponent()->GetOwner());

	//--------------------------------------------------
	// 0. 墙面坐标系无效（Activate 未找到墙）→ 尝试补救
	//--------------------------------------------------
	if (WallNormal.IsNearlyZero())
	{
		const FVector Fwd = UpdatedComp->GetForwardVector();
		FHitResult RescueHit;
		const bool bRescue = GetWorld()->LineTraceSingleByChannel(
			RescueHit, StartingLocation, StartingLocation + Fwd * WallSearchDistance,
			ECC_WorldStatic, QueryParams);

		if (bRescue && FVector::DotProduct(RescueHit.Normal, FVector::UpVector) <= 0.75f)
		{
			UpdateWallBasis(RescueHit.Normal);
			const FVector TargetPos = RescueHit.Location + WallNormal * StickDistance;
			UpdatedComp->MoveComponent(TargetPos - StartingLocation, CurrentRotation, true, nullptr);
			const FRotator Rot = FRotationMatrix::MakeFromXY(WallUp, -WallRight).Rotator();
			UpdatedComp->SetWorldRotation(Rot);
			CurrentRotation = Rot.Quaternion();
		}
		else
		{
			OutputState.MovementEndState.NextModeName = LrAllModes::Air;
			OutputSyncState.SetTransforms_WorldSpace(
				StartingLocation, CurrentRotation.Rotator(),
				FVector::ZeroVector, FVector::ZeroVector);
			return;
		}
	}

	//--------------------------------------------------
	// 1. 按提议速度移动
	//--------------------------------------------------
	FVector MoveDelta = Params.ProposedMove.LinearVelocity * DeltaTime;
	FHitResult MoveHit;
	UpdatedComp->MoveComponent(MoveDelta, CurrentRotation, true, &MoveHit);

	//--------------------------------------------------
	// 2. 中心射线：保持腹部贴墙
	//    从角色位置沿 WallNormal 偏移 5cm 开始，避免射线起点在墙内
	//--------------------------------------------------
	FVector CurPos = UpdatedComp->GetComponentLocation();
	const FVector RayStart = CurPos + WallNormal * 5.f;
	FHitResult CenterHit;
	const bool bCenterHit = GetWorld()->LineTraceSingleByChannel(
		CenterHit, RayStart, RayStart - WallNormal * (ProbeLength + 5.f), ECC_WorldStatic, QueryParams);

	bool bOnWall = bCenterHit;

	if (bCenterHit)
	{
		// 更新墙面坐标系
		UpdateWallBasis(CenterHit.Normal);

		// 吸附到墙面
		const FVector DesiredPos = CenterHit.Location + WallNormal * StickDistance;
		const FVector SnapDelta = DesiredPos - CurPos;
		if (!SnapDelta.IsNearlyZero())
		{
			FHitResult SnapHit;
			UpdatedComp->MoveComponent(SnapDelta, CurrentRotation, true, &SnapHit);
			CurPos = UpdatedComp->GetComponentLocation();
		}
	}
	else
	{
		// 中心射线未命中 → 尝试4根探空射线寻找墙面
		bool bFoundViaProbe = false;
		const FVector ProbeDirs[4] = { WallUp, -WallUp, -WallRight, WallRight };

		for (int32 i = 0; i < 4; ++i)
		{
			const FVector ProbeStart = CurPos + ProbeDirs[i] * ProbeOffset;
			const FVector ProbeEnd = ProbeStart - WallNormal * ProbeLength;

			FHitResult ProbeHit;
			if (GetWorld()->LineTraceSingleByChannel(ProbeHit, ProbeStart, ProbeEnd, ECC_WorldStatic, QueryParams))
			{
				UpdateWallBasis(ProbeHit.Normal);
				const FVector DesiredPos = ProbeHit.Location + WallNormal * StickDistance;
				const FVector SnapDelta = DesiredPos - CurPos;
				if (!SnapDelta.IsNearlyZero())
				{
					FHitResult SnapHit;
					UpdatedComp->MoveComponent(SnapDelta, CurrentRotation, true, &SnapHit);
					CurPos = UpdatedComp->GetComponentLocation();
				}
				bFoundViaProbe = true;
				bOnWall = true;
				break;
			}
		}

		if (!bFoundViaProbe)
		{
			// 完全找不到墙 → 切换到 Air
			OutputState.MovementEndState.NextModeName = LrAllModes::Air;
			OutputSyncState.SetTransforms_WorldSpace(
				UpdatedComp->GetComponentLocation(), CurrentRotation.Rotator(),
				Params.ProposedMove.LinearVelocity, FVector::ZeroVector);
			return;
		}
	}

	//--------------------------------------------------
	// 3. 4根探空射线 + 折返射线（检测墙面边缘 / 绕角）
	//--------------------------------------------------
	CurPos = UpdatedComp->GetComponentLocation();
	int32 MissCount = 0;
	FVector NewWallNormal = FVector::ZeroVector;
	bool bNewWallFound = false;

	const FVector ProbeDirs[4] = { WallUp, -WallUp, -WallRight, WallRight };

	for (int32 i = 0; i < 4; ++i)
	{
		const FVector ProbeStart = CurPos + ProbeDirs[i] * ProbeOffset;
		const FVector ProbeEnd = ProbeStart - WallNormal * ProbeLength;

		FHitResult ProbeHit;
		const bool bProbeHit = GetWorld()->LineTraceSingleByChannel(
			ProbeHit, ProbeStart, ProbeEnd, ECC_WorldStatic, QueryParams);

		// 调试：探空射线
		if (bDrawDebug)
		{
			DrawDebugLine(GetWorld(), ProbeStart, ProbeEnd,
			              bProbeHit ? FColor::Green : FColor::Red, false, -1.f, 0, 1.f);
		}

		if (!bProbeHit)
		{
			MissCount++;

			// 折返射线：从探空射线末端沿探空方向的反方向射出，寻找转角处的新墙面
			const FVector BackEnd = ProbeEnd - ProbeDirs[i] * BackRayLength;

			FHitResult BackHit;
			const bool bBackHit = GetWorld()->LineTraceSingleByChannel(
				BackHit, ProbeEnd, BackEnd, ECC_WorldStatic, QueryParams);

			// 调试：折返射线
			if (bDrawDebug)
			{
				DrawDebugLine(GetWorld(), ProbeEnd, BackEnd,
				              bBackHit ? FColor::Cyan : FColor::Orange, false, -1.f, 0, 1.f);
			}

			if (bBackHit)
			{
				// 确保命中的是不同墙面（法线与当前墙面差异足够大）
				const float BackNormalDot = FMath::Abs(FVector::DotProduct(BackHit.Normal, WallNormal));
				if (BackNormalDot < 0.7f)
				{
					NewWallNormal = BackHit.Normal;
					bNewWallFound = true;

					// 调试：新墙面目标球体
					if (bDrawDebug)
					{
						DrawDebugSphere(GetWorld(), BackHit.ImpactPoint, 8.f, 8,
						                FColor::Magenta, false, -1.f, 0, 2.f);
					}
				}
			}
		}
	}

	// 所有探空射线均未命中且折返射线也未找到新墙 → Air
	if (MissCount == 4 && !bNewWallFound)
	{
		OutputState.MovementEndState.NextModeName = LrAllModes::Air;
		OutputSyncState.SetTransforms_WorldSpace(
			UpdatedComp->GetComponentLocation(), CurrentRotation.Rotator(),
			Params.ProposedMove.LinearVelocity, FVector::ZeroVector);
		return;
	}

	// 折返射线找到新墙面 → 绕角，更新坐标系并重新吸附
	// 仅当新法线与当前法线差异足够大时才切换，防止抖动
	if (bNewWallFound)
	{
		const float NormalDot = FMath::Abs(FVector::DotProduct(WallNormal, NewWallNormal));
		if (NormalDot < 0.866f) // 约30°阈值
		{
			UpdateWallBasis(NewWallNormal);

			// 先更新旋转，再做 re-snap（避免用旧旋转移动导致方向错误）
			const FRotator NewRot = FRotationMatrix::MakeFromXY(WallUp, -WallRight).Rotator();
			CurrentRotation = NewRot.Quaternion();
			UpdatedComp->SetWorldRotation(CurrentRotation);

			CurPos = UpdatedComp->GetComponentLocation();
			FHitResult ReCenterHit;
			if (GetWorld()->LineTraceSingleByChannel(
				ReCenterHit, CurPos, CurPos - WallNormal * ProbeLength, ECC_WorldStatic, QueryParams))
			{
				const FVector DesiredPos = ReCenterHit.Location + WallNormal * StickDistance;
				const FVector SnapDelta = DesiredPos - CurPos;
				if (!SnapDelta.IsNearlyZero())
				{
					FHitResult SnapHit;
					UpdatedComp->MoveComponent(SnapDelta, CurrentRotation, true, &SnapHit);
				}
			}
		}
	}

	//--------------------------------------------------
	// 4. 旋转：处理攻击转身锁 vs 攀爬朝向
	//--------------------------------------------------
	bool bHasExternalLock = false;
	FRotator ExternalRot = FRotator::ZeroRotator;
	if (CacheMoverComponent)
	{
		bHasExternalLock = CacheMoverComponent->bIsInAttackWarp;
		if (bHasExternalLock)
		{
			ExternalRot = CacheMoverComponent->AttackWarpRotation;
		}
	}

	if (bHasExternalLock)
	{
		CurrentRotation = ExternalRot.Quaternion();
	}
	else
	{
		CurrentRotation = FRotationMatrix::MakeFromXY(WallUp, -WallRight).ToQuat();
	}
	UpdatedComp->SetWorldRotation(CurrentRotation);

	//--------------------------------------------------
	// 5. 调试：向前射线 + 中心射线
	//--------------------------------------------------
	if (bDrawDebug)
	{
		CurPos = UpdatedComp->GetComponentLocation();
		// 向前 80 cm 射线（搜索方向）
		DrawDebugLine(GetWorld(), CurPos, CurPos + WallNormal * WallSearchDistance,
		              FColor::Yellow, false, -1.f, 0, 1.f);
		// 中心射线（贴墙检测）
		DrawDebugLine(GetWorld(), CurPos, CurPos - WallNormal * ProbeLength,
		              bOnWall ? FColor::Blue : FColor::Red, false, -1.f, 0, 2.f);
	}

	//--------------------------------------------------
	// 6. 计算实际速度并输出
	//--------------------------------------------------
	const FVector FinalLocation = UpdatedComp->GetComponentLocation();
	const FVector ActualVelocity = (FinalLocation - StartingLocation) / DeltaTime;

	OutputSyncState.SetTransforms_WorldSpace(
		FinalLocation,
		CurrentRotation.Rotator(),
		ActualVelocity,
		FVector::ZeroVector);
}
