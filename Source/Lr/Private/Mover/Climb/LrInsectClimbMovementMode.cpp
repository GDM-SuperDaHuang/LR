#include "Mover/Climb/LrInsectClimbMovementMode.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "MoverComponent.h"
#include "Mover/FLrMoverInputCmd.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMoverComponent.h"

ULrInsectClimbMovementMode::ULrInsectClimbMovementMode()
{
}


//==============================================================================
// Activate — 进入攀爬模式时的初始吸附
//   1) 沿角色前方向前发射 WallSearchDistance(40cm) 射线寻找墙面
//   2) 若直线未命中，改用球体扫描作为容错
//   3) 拒绝过于平坦的表面（与地面法线夹角 < 25°）
//   4) 建立墙面局部坐标系：Z=WallNormal(腹部朝外), X=WallForward(头部朝上), Y=-WallRight(左右)
//   5) 沿法线将角色吸附到墙面 + 立即设置旋转
//==============================================================================
void ULrInsectClimbMovementMode::Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext,
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
	// 1. 向前 40cm 射线找墙
	//--------------------------------------------------
	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, Start + Forward * WallSearchDistance, ECC_WorldStatic, QueryParams);

	// 直线未命中 → 球体扫描作为fallback（处理角色嵌入墙内的情况）
	if (!bHit)
	{
		const FCollisionShape Sphere = FCollisionShape::MakeSphere(WallSearchDistance * 0.5f);
		bHit = GetWorld()->SweepSingleByChannel(
			Hit, Start, Start + Forward * WallSearchDistance, FQuat::Identity,
			ECC_WorldStatic, Sphere, QueryParams);
	}

	if (!bHit) return;

	// 拒绝过于平坦的表面（与地面的夹角 < 25°，Dot(法线, Up) > cos25° ≈ 0.906）
	if (FVector::DotProduct(Hit.Normal, FVector::UpVector) > 0.75f) return;

	// 建立墙面局部坐标系
	UpdateWallBasis(Hit.Normal, Forward);

	// 沿法线吸附到墙面，腹部贴墙
	const FVector TargetPos = Hit.Location + WallNormal * StickDistance;
	const FVector SnapDelta = TargetPos - Start;
	if (!SnapDelta.IsNearlyZero())
	{
		FHitResult SnapHit;
		UpdatedComp->MoveComponent(SnapDelta, UpdatedComp->GetComponentQuat(), true, &SnapHit);
	}

	// 旋转：MakeFromXY(X=WallForward, Y=-WallRight) → Z=CrossProduct(X,Y)=WallNormal
	// 昆虫头部（X轴）朝上，腹部（Z轴）朝墙外
	const FRotator DesiredRotation = FRotationMatrix::MakeFromXY(WallForward, -WallRight).Rotator();
	UpdatedComp->SetWorldRotation(DesiredRotation);

	UpdateWallRotationBasis(Hit.Normal, UpdatedComp->GetComponentQuat(), Forward);
}

//==============================================================================
// GenerateMove — 将玩家输入（A/D、W/S）映射到墙面局部坐标系
//   • 不依赖相机朝向，直接使用墙面局部坐标（与 UpdateWallBasis 一致）
//   • A/D → WallRight（墙面左右轴），W/S → WallForward（墙面上下/前后轴）
//   • 最终速度 = WallFinalMove * ClimbSpeed
//==============================================================================
void ULrInsectClimbMovementMode::GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState,
                                                             const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	Super::GenerateMove_Implementation(SimContext, StartState, TimeStep, OutProposedMove);

	const FLrMoverInputCmd* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();
	if (!Inputs) return;

	const FVector MoveInput = Inputs->GetMoveInput();
	// 不依赖相机朝向：直接使用墙面局部坐标，A/D→-WallRight，W/S→WallForward
	OutProposedMove.LinearVelocity = WallFinalMove * ClimbSpeed;
	OutProposedMove.DirectionIntent = MoveInput;
}

//==============================================================================
// SimulationTick — 每帧墙面检测 + 移动 + 边缘检测 + 绕角
//
//   射线布局（共 5 组）:
//     ① 中心射线(向墙内 50cm) — 检测墙面法线、吸附腹部
//     ② 背部射线(向墙外 200cm) — 检测背部墙面（起跳 TODO）
//     ③ 4方向射线(沿墙面上/下/左/右 40cm) — 中心未命中时寻找相邻墙面
//     ④ 4探空射线(向墙内 50cm) — 检测是否越过墙面边缘
//     ⑤ 4折返射线(探空末端→角色中心，去法线分量) — 绕角寻找新墙面
//
//   决策链：中心命中→方向射线→折返射线→切 Air
//==============================================================================
void ULrInsectClimbMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
	const FMoverDefaultSyncState* StartingSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	if (!StartingSyncState) return;

	TWeakObjectPtr<USceneComponent> UpdatedComp = Params.MovingComps.UpdatedComponent;
	if (!UpdatedComp.Get()) return;

	const float DeltaTime = Params.TimeStep.StepMs * 0.001f;
	const FVector StartingLocation = UpdatedComp->GetComponentLocation();
	FQuat CurrentRotation = StartingSyncState->GetOrientation_WorldSpace().Quaternion();

	//排除自身
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetMoverComponent()->GetOwner());

	// 读取玩家输入：用于 UpdateWallBasis 的基向量计算 + 绕角意图判断
	// 默认给 (0,1,0) 确保垂直墙面也能正确计算叉乘基向量
	const FLrMoverInputCmd* SimInputs = Params.StartState.InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();
	FVector RawMoveInput = FVector(0, 1, 0);
	if (SimInputs)
	{
		RawMoveInput = SimInputs->GetMoveInput();
		if (!RawMoveInput.IsNearlyZero()) RawMoveInput = RawMoveInput.GetSafeNormal();
	}

	//--------------------------------------------------
	// 0.5 绕角过渡：向新墙平滑滑动（位置+旋转插值，替代瞬移）
	//--------------------------------------------------
	if (bWallTransition)
	{
		TransitionAlpha = FMath::Min(1.f, TransitionAlpha + DeltaTime / WallTransitionTime);
		const float SmoothA = FMath::SmoothStep(0.f, 1.f, TransitionAlpha);
		const FVector LerpedPos = FMath::Lerp(TransitionStartPos, TransitionTargetPos, SmoothA);
		const FQuat LerpedRot = FQuat::Slerp(TransitionStartRot, TransitionTargetRot, SmoothA);
		UpdatedComp->SetWorldLocationAndRotation(LerpedPos, LerpedRot);
		if (TransitionAlpha >= 1.f)
		{
			// 过渡完成：采用新墙面基向量
			bWallTransition = false;
			UpdateWallBasis(TransitionTargetNormal, RawMoveInput);
		}
		const FVector TransFinal = UpdatedComp->GetComponentLocation();
		const FVector TransVel = (TransFinal - StartingLocation) / DeltaTime;
		OutputSyncState.SetTransforms_WorldSpace(TransFinal, UpdatedComp->GetComponentQuat().Rotator(), TransVel, FVector::ZeroVector);
		return;
	}

	//--------------------------------------------------
	// 0. 未找到墙直接下落
	//--------------------------------------------------
	if (WallNormal.IsNearlyZero())
	{
		OutputState.MovementEndState.NextModeName = LrAllModes::Air;
		return;
	}

	//--------------------------------------------------
	// 1. 按 GenerateMove 提议的速度移动（物理位移）
	//--------------------------------------------------
	FVector MoveDelta = Params.ProposedMove.LinearVelocity * DeltaTime;
	FHitResult MoveHit;
	UpdatedComp->MoveComponent(MoveDelta, CurrentRotation, true, &MoveHit);
	FVector CurPos = UpdatedComp->GetComponentLocation();

	//是否发现新墙
	TArray<FLrWallInfo> FLrNewWallInfos;

	// FVector NewWallNormal = FVector::ZeroVector;
	// FVector NewWallImpactPoint = FVector::ZeroVector;
	FVector NewWallImpactDir = FVector::ZeroVector; //用于检查玩家移动意图是否朝向新墙面
	bool bNewWallFound = false; //是否发现新墙
	FVector CurWallNormal = WallNormal; //当前墙
	//==================================================================
	// 2a. 中心射线（向墙内 CenterRayLength=50cm）
	//     起点沿 WallNormal 偏移 5cm 避免起点嵌在墙内
	//     命中时：更新墙面法线 + 吸附腹部到贴墙距离
	//==================================================================
	bool bOnWall = false;
	{
		const FVector RayStart = CurPos + WallNormal * 5.f;
		FHitResult CenterHit;
		const bool bCenterHit = GetWorld()->LineTraceSingleByChannel(
			CenterHit, RayStart, RayStart - WallNormal * CenterRayLength,
			ECC_WorldStatic, QueryParams);

		if (bCenterHit)
		{
			bOnWall = true; // 标记仍在墙上
			UpdateWallBasis(CenterHit.Normal, RawMoveInput); // 更新墙面基向量
			// 沿最新法线吸附到 StickDistance 距离
			const FVector DesiredPos = CenterHit.Location + WallNormal * StickDistance;
			const FVector SnapDelta = DesiredPos - CurPos;
			if (!SnapDelta.IsNearlyZero())
			{
				FHitResult SnapHit;
				UpdatedComp->MoveComponent(SnapDelta, CurrentRotation, true, &SnapHit);
				CurPos = UpdatedComp->GetComponentLocation();
			}
		}
	}

	//==================================================================
	// 2b. 背部射线（向墙外 BackRayLength=200cm）
	//     检测角色背部的墙面，用于将来实现踩墙跳（TODO：暂不实现）
	//==================================================================
	FHitResult BackHit;
	const bool bBackHit = GetWorld()->LineTraceSingleByChannel(BackHit, CurPos, CurPos + WallNormal * BackRayLength, ECC_WorldStatic, QueryParams);

	//==================================================================
	// 3. 4方向射线（沿墙面向上/下/左/右 DirectionRayLength=40cm）
	//     当中心射线未命中时，检查周围是否有相邻墙面可以吸附
	//     方向：WallForward(上), -WallForward(下), -WallRight(左), WallRight(右)
	//==================================================================
	const FVector DirDirs[4] = {WallForward, -WallForward, -WallRight, WallRight};
	// FVector DirWallNormal = FVector::ZeroVector;
	// FVector DirWallImpact = FVector::ZeroVector;
	// bool bFoundWallViaDir = false;

	//可能遇到凸出来的墙
	for (int32 i = 0; i < 4; ++i)
	{
		const FVector DirStart = CurPos + DirDirs[i] * 5.f;
		const FVector DirEnd = DirStart + DirDirs[i] * DirectionRayLength;

		FHitResult DirHit;
		const bool bDirHit = GetWorld()->LineTraceSingleByChannel(DirHit, DirStart, DirEnd, ECC_WorldStatic, QueryParams);

		const float DirNUpDot = bDirHit ? FVector::DotProduct(DirHit.Normal, FVector::UpVector) : 1.f;
		// 墙面任意方向都接受；法线朝上的面（墙顶/平台）仅上方向接受，用于到达顶部后切 Walk
		if (bDirHit && (DirNUpDot <= 0.75f || (i == 0 && DirNUpDot > 0.75f)))
		{
			// 找到有效墙面 → 记录法线和命中点
			// DirWallNormal = DirHit.Normal;
			// DirWallImpact = DirHit.Location;
			// bFoundWallViaDir = true;

			FLrWallInfo FLrWallInfo;
			FLrWallInfo.WallNormal = DirHit.Normal;
			FLrWallInfo.WallImpactPoint = DirHit.Location;
			FLrNewWallInfos.Add(FLrWallInfo);

			// 中心未命中但方向射线找到墙 → 吸附到该墙面
			// NewWallNormal = DirWallNormal;
			// NewWallImpactPoint = DirWallImpact;
			NewWallImpactDir = (DirHit.ImpactPoint - CurPos).GetSafeNormal();
			bNewWallFound = true;

			if (bDrawDebug) DrawDebugSphere(GetWorld(), DirHit.ImpactPoint, 8.f, 8, FColor::Cyan, false, -1.f, 0, 2.f);

			break;
		}
	}


	//==================================================================
	// 4. 4根探空射线 + 4根折返射线（检测墙面边缘 → 绕角过渡）
	//
	//   探空射线：从角色中心向上/下/左/右偏移40cm → 向墙内(-WallNormal)射 ProbeLength
	//     - 命中(绿)：还在墙面上
	//     - 未命中(红)：已越过墙面边缘 → 发射折返射线
	//
	//   折返射线：从探空末端向角色中心方向（去除 WallNormal 分量）射 EdgeReturnLength
	//     - 命中(青)：找到新墙面 → 记入候选；绘制品红球体
	//     - 未命中(橙)：没有新墙面
	//==================================================================
	int32 MissCount = 0;
	{
		const FVector ProbeDirs[4] = {WallForward, -WallForward, -WallRight, WallRight};
		for (int32 i = 0; i < 4; ++i)
		{
			// 探空射线：沿 ProbeDir 偏移 40cm → 向墙内射 ProbeLength
			const FVector ProbeStart = CurPos + ProbeDirs[i] * 40.f;
			const FVector ProbeEnd = ProbeStart - WallNormal * ProbeLength;

			FHitResult ProbeHit;
			const bool bProbeHit = GetWorld()->LineTraceSingleByChannel(ProbeHit, ProbeStart, ProbeEnd, ECC_WorldStatic, QueryParams);

			if (bDrawDebug) // 调试：探空射线
			{
				DrawDebugLine(GetWorld(), ProbeStart, ProbeEnd, bProbeHit ? FColor::Green : FColor::Red, false, -1.f, 0, 1.f);
			}

			if (!bProbeHit) // 探空未命中,刚刚越过边缘,进入折返射线
			{
				// 探空未命中 → 已越过边缘
				MissCount++;

				// 折返方向：探空末端 → 角色中心，投影到墙面切平面（去掉 WallNormal 分量）
				// const FVector ToCenter = (CurPos - ProbeEnd).GetSafeNormal();
				// const FVector ReturnDir = (ToCenter - FVector::ProjectOnToNormal(ToCenter, WallNormal)).GetSafeNormal();
				// const FVector ReturnEnd = ProbeEnd + ReturnDir * EdgeReturnLength;

				const FVector ReturnEnd = ProbeEnd - ProbeDirs[i] * EdgeReturnLength;

				FHitResult ReturnHit;
				const bool bReturnHit = GetWorld()->LineTraceSingleByChannel(
					ReturnHit,
					ProbeEnd,
					ReturnEnd,
					ECC_WorldStatic,
					QueryParams);

				if (bDrawDebug) // 调试：折返射线
				{
					DrawDebugLine(GetWorld(), ProbeEnd, ReturnEnd, bReturnHit ? FColor::Cyan : FColor::Orange, false, -1.f, 0, 1.f);
				}

				if (bReturnHit)
				{
					// 是否是新墙 值为 1.0：两个法线完全平行,值为 0.0：两个法线完全垂直，确认是新墙面（法线与当前墙面差异 > 45°，cos45°≈0.7）
					const float NormalDot = FMath::Abs(FVector::DotProduct(ReturnHit.Normal, WallNormal));
					if (NormalDot < 0.7f)
					{
						FLrWallInfo FLrWallInfo;
						FLrWallInfo.WallNormal = ReturnHit.Normal;
						FLrWallInfo.WallImpactPoint = ReturnHit.Location;
						FLrNewWallInfos.Add(FLrWallInfo);

						// NewWallNormal = ReturnHit.Normal;
						// NewWallImpactPoint = ReturnHit.Location;
						NewWallImpactDir = (ReturnHit.ImpactPoint - CurPos).GetSafeNormal();
						bNewWallFound = true;

						if (bDrawDebug)
						{
							DrawDebugSphere(GetWorld(), ReturnHit.ImpactPoint, 8.f, 8, FColor::Magenta, false, -1.f, 0, 2.f);
						}
					}
				}
			}
		}
	}

	//==================================================================
	// 5. 墙面状态决策（链式回退）
	//   ① 中心命中 → bOnWall
	//   ② 方向射线命中 → 吸附到方向墙面
	//   ③ 折返射线找到新墙 → 传送到新墙面
	//   ④ 全部未命中且无候选 → 切 Air
	//==================================================================
	if (!bOnWall && FLrNewWallInfos.Num() > 0)
	{
		NewWallImpactDir = (DirHit.ImpactPoint - CurPos).GetSafeNormal();
		// 中心未命中但方向射线找到墙 → 吸附到该墙面
		UpdateWallBasis(DirWallNormal, RawMoveInput);
		const FVector DesiredPos = DirWallImpact + WallNormal * StickDistance;
		const FVector SnapDelta = DesiredPos - CurPos;
		if (!SnapDelta.IsNearlyZero())
		{
			FHitResult SnapHit;
			UpdatedComp->MoveComponent(SnapDelta, CurrentRotation, true, &SnapHit);
			CurPos = UpdatedComp->GetComponentLocation();
		}
		bOnWall = true;
	}

	if (!bOnWall)
	{
		if (bNewWallFound)
		{
			// 折返射线找到新墙面 → 平滑滑动到新墙（替代直接传送）
			BeginWallTransition(NewWallNormal, NewWallImpactPoint);
			bOnWall = true;
		}
		else if (MissCount == 4 && !bNewWallFound)
		{
			// 四面探空全未命中 + 折返未找到新墙 → 完全离开墙面 → Air
			OutputState.MovementEndState.NextModeName = LrAllModes::Air;
			OutputSyncState.SetTransforms_WorldSpace(
				UpdatedComp->GetComponentLocation(), CurrentRotation.Rotator(),
				Params.ProposedMove.LinearVelocity, FVector::ZeroVector);
			return;
		}
	}

	// 折返射线找到新墙面 → 当玩家移动指向新墙面时绕角切换
	if (bNewWallFound && bOnWall && SimInputs)
	{
		const FVector SimMoveInput = SimInputs->GetMoveInput();
		// 不依赖相机朝向：与 UpdateWallBasis 保持一致，A/D→-WallRight，W/S→WallForward
		const FVector WallMoveDir = (-SimMoveInput.X * WallRight + SimMoveInput.Y * WallForward).GetSafeNormal();

		const float NormalDot = FMath::Abs(FVector::DotProduct(WallNormal, NewWallNormal));
		const float IntentDot = FVector::DotProduct(WallMoveDir, NewWallImpactDir);
		// 玩家移动指向新墙面 → 开始平滑绕角过渡
		if (NormalDot < 0.866f && IntentDot > 0.3f)
		{
			BeginWallTransition(NewWallNormal, NewWallImpactPoint);
		}
	}

	// 已开始绕角过渡：本帧直接输出，由下一帧的过渡逻辑接管
	if (bWallTransition)
	{
		OutputSyncState.SetTransforms_WorldSpace(
			UpdatedComp->GetComponentLocation(), CurrentRotation.Rotator(),
			Params.ProposedMove.LinearVelocity, FVector::ZeroVector);
		return;
	}

	//--------------------------------------------------
	// 6. 旋转：昆虫腹部朝向墙外（Z=WallNormal）
	//--------------------------------------------------
	UpdateWallRotationBasis(WallNormal, CurrentRotation, RawMoveInput);
	// 同时需要重新获取当前的 CurrentRotation 用于后续的输出
	CurrentRotation = UpdatedComp->GetComponentQuat();

	//--------------------------------------------------
	// 6.5 到达法线朝上的面（类似地面的平台/墙顶）→ 切换到 Walk 模式
	//--------------------------------------------------
	if (FVector::DotProduct(WallNormal, FVector::UpVector) > 0.75f)
	{
		OutputState.MovementEndState.NextModeName = LrAllModes::Walk;
		OutputSyncState.SetTransforms_WorldSpace(
			UpdatedComp->GetComponentLocation(), CurrentRotation.Rotator(),
			Params.ProposedMove.LinearVelocity, FVector::ZeroVector);
		return;
	}

	//--------------------------------------------------
	// 7. 调试绘制
	//--------------------------------------------------
	if (bDrawDebug)
	{
		const FVector DebugPos = UpdatedComp->GetComponentLocation();

		DrawDebugLine(GetWorld(), DebugPos, DebugPos - WallNormal * CenterRayLength,
		              bOnWall ? FColor::Blue : FColor::Red, false, -1.f, 0, 2.f);

		DrawDebugLine(GetWorld(), DebugPos, DebugPos + WallNormal * BackRayLength,
		              bBackHit ? FColor::Yellow : FColor::Silver, false, -1.f, 0, 1.f);

		for (int32 i = 0; i < 4; ++i)
		{
			const FVector DStart = DebugPos + DirDirs[i] * 5.f;
			const FVector DEnd = DStart + DirDirs[i] * DirectionRayLength;
			DrawDebugLine(GetWorld(), DStart, DEnd, FColor::White, false, -1.f, 0, 1.f);
		}
	}

	//--------------------------------------------------
	// 8. 计算实际速度并输出
	//--------------------------------------------------
	const FVector FinalLocation = UpdatedComp->GetComponentLocation();
	const FVector ActualVelocity = (FinalLocation - StartingLocation) / DeltaTime;

	OutputSyncState.SetTransforms_WorldSpace(
		FinalLocation,
		CurrentRotation.Rotator(),
		ActualVelocity,
		FVector::ZeroVector);
}

void ULrInsectClimbMovementMode::UpdateWallBasis(const FVector& InWallNormal, const FVector& MoveInput)
{
	WallNormal = InWallNormal.GetSafeNormal();
	// 1. 始终计算 WallRight 和 WallForward（依赖法线，作为头部朝向兜底）
	if (InWallNormal == FVector(0, 0, -1) || InWallNormal == FVector(0, 0, 1))
	{
		WallRight = FVector(1, 0, 0);
		WallForward = FVector(0, -1, 0);
	}
	else
	{
		WallRight = FVector::CrossProduct(FVector::UpVector, InWallNormal).GetSafeNormal();
		WallForward = FVector::CrossProduct(InWallNormal, WallRight).GetSafeNormal();
	}

	// 2. 头部朝向：固定使用墙面基向量 WallForward（不依赖相机）
	HeadDir = WallForward;

	// 3. 计算最终移动向量（保留输入方向，但输入为零时置零）
	if (MoveInput != FVector::ZeroVector)
	{
		// 角色 forward = HeadDir（W/S 跟随头部朝向），right = WallNormal × HeadDir（A/D 随之改变）
		const FVector WallRightDir = FVector::CrossProduct(WallNormal, HeadDir).GetSafeNormal();
		WallFinalMove = (MoveInput.X * WallRightDir) + (MoveInput.Y * HeadDir) + (MoveInput.Z * InWallNormal);
	}
	else
	{
		WallFinalMove = FVector::ZeroVector;
	}
	if (WallFinalMove != FVector::ZeroVector)
	{
		UE_LOG(LogTemp, Display, TEXT("=== WallFinalMove (%f,%f,%f)"), WallFinalMove.X, WallFinalMove.Y, WallFinalMove.Z);
	}
}

void ULrInsectClimbMovementMode::UpdateWallRotationBasis(const FVector& InWallNormal, FQuat CurrentRotation, FVector MoveInput)
{
	// 确保基向量已更新（一般调用前已通过 UpdateWallBasis 计算过）
	FRotator DesiredRotation;
	if (FVector::DotProduct(WallNormal, FVector::UpVector) > 0.75f)
	{
		// 水平朝上的面（地板/立方体顶部）：脚贴地站立姿势
		// 保持当前 Yaw 朝向，Pitch/Roll 归零使 Z=世界Up（背部朝上）
		DesiredRotation = CurrentRotation.Rotator();
		DesiredRotation.Pitch = 0.f;
		DesiredRotation.Roll = 0.f;
	}
	else
	{
		// 墙面/天花板：头部朝向固定为墙面基向量 HeadDir
		FVector UseHeadDir = HeadDir;
		if (UseHeadDir.IsNearlyZero())
			UseHeadDir = WallForward;

		// X = HeadDir（头部朝向），Y = WallNormal × HeadDir（左右），Z = WallNormal（腹部朝外）
		DesiredRotation = FRotationMatrix::MakeFromXY(
			UseHeadDir, FVector::CrossProduct(WallNormal, UseHeadDir)).Rotator();
	}

	// 如果需要根据 MoveInput 调整朝向（例如让角色面朝移动方向），可在此扩展。
	// 当前设计保持墙面固定坐标系，不随输入旋转。

	USceneComponent* UpdatedComp = GetMoverComponent()->GetUpdatedComponent();
	if (UpdatedComp)
	{
		UpdatedComp->SetWorldRotation(DesiredRotation);
	}
}

void ULrInsectClimbMovementMode::BeginWallTransition(const FVector& NewNormal, const FVector& ImpactPoint)
{
	USceneComponent* UpdatedComp = GetMoverComponent()->GetUpdatedComponent();
	if (!UpdatedComp) return;

	const FVector TargetNormal = NewNormal.GetSafeNormal();
	const FVector TargetPos = ImpactPoint + TargetNormal * StickDistance;

	// 计算新墙面朝向（X=HeadDir，固定为墙面基向量 WallForward；Z=新墙法线）
	FVector Right, Forward;
	if (TargetNormal == FVector(0, 0, -1) || TargetNormal == FVector(0, 0, 1))
	{
		Right = FVector(1, 0, 0);
		Forward = FVector(0, -1, 0);
	}
	else
	{
		Right = FVector::CrossProduct(FVector::UpVector, TargetNormal).GetSafeNormal();
		Forward = FVector::CrossProduct(TargetNormal, Right).GetSafeNormal();
	}
	const FQuat TargetRot = FRotationMatrix::MakeFromXY(Forward, FVector::CrossProduct(TargetNormal, Forward)).Rotator().Quaternion();

	bWallTransition = true;
	TransitionTargetNormal = TargetNormal;
	TransitionTargetPos = TargetPos;
	TransitionTargetRot = TargetRot;
	TransitionStartPos = UpdatedComp->GetComponentLocation();
	TransitionStartRot = UpdatedComp->GetComponentQuat();
	TransitionAlpha = 0.f;

	UE_LOG(LogTemp, Display, TEXT("=== BeginWallTransition -> (%f,%f,%f)"), TargetNormal.X, TargetNormal.Y, TargetNormal.Z);
}
