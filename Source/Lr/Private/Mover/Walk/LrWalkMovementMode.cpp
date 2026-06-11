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

void ULrWalkMovementMode::Activate()
{
	Super::Activate();
	if (!CacheMoverComponent)
	{
		if (ULrMoverComponent* Mover = Cast<ULrMoverComponent>(GetMoverComponent()))
		{
			CacheMoverComponent = Mover;
		}
	}
}

void ULrWalkMovementMode::GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	const FLrMoverInputCmd* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();

	if (!SyncState) return;

	// 安全保底：若无输入数据，强制视作静止制动
	if (!Inputs)
	{
		OutProposedMove.LinearVelocity = FVector(0.0f, 0.0f, SyncState->GetVelocity_WorldSpace().Z);
		OutProposedMove.DirectionIntent = FVector::ZeroVector;
		return;
	}

	const ULrMovementSettings* Settings = nullptr;
	if (const ULrMoverComponent* MyMover = Cast<ULrMoverComponent>(GetMoverComponent()))
	{
		Settings = MyMover->RealisticSettings;
	}
	if (!Settings)
	{
		Settings = GetMoverComponent()->FindSharedSettings<ULrMovementSettings>();
	}

	float DeltaSeconds = TimeStep.StepMs * 0.001f;
	FVector CurrentVelocity = SyncState->GetVelocity_WorldSpace();
	if (CurrentVelocity.Size() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT(""));
	}
	float StrippedZVelocity = CurrentVelocity.Z;
	CurrentVelocity.Z = 0.0f;

	// 读取输入
	FVector MoveInput = Inputs->GetMoveInput();
	float InputMagnitude = MoveInput.Size();

	FVector MoveIntent = FVector::ZeroVector;
	float SpeedScale = 0.0f;

	// 【输入安全边界】
	const float SafeDeadzone = 0.20f;
	if (InputMagnitude > SafeDeadzone)
	{
		MoveIntent = MoveInput.GetSafeNormal();
		SpeedScale = FMath::Clamp((InputMagnitude - SafeDeadzone) / (1.0f - SafeDeadzone), 0.0f, 1.0f);
	}

	// ————————————————————————————————————————————————————————————————
	// 【核心物理参数定义：通过极端数值欺骗人眼，达成“有物理的0惯性”】
	// ————————————————————————————————————————————————————————————————
	const float TargetMaxSpeed = (Settings ? Settings->MaxWalkSpeed : 600.0f) * (Inputs->bIsCrouchPressed ? 0.5f : (Inputs->bIsBlinkPressed ? 1.6f : 1.0f));
	const float StableFriction = 12.0f; // 稳定的高额地面摩擦力：网络稳定的定海神针
	const float InfiniteAcceleration = 25000.0f; // 恐怖的爆发加速度：0.02秒内充满速度，体感绝对0起步惯性
	const float InstantBraking = 5000.0f; // 极强的刹车制动减速度：松手瞬间死死钉在原地，绝不滑行

	if (SpeedScale > 0.0f)
	{
		// 【去转弯惯性优化】如果玩家推反方向摇杆（角度大于90度），瞬间蒸发旧速度，实现无半径折返
		if ((CurrentVelocity | MoveIntent) < 0.0f)
		{
			CurrentVelocity = FVector::ZeroVector;
		}

		// 1. 先应用高额物理摩擦力（让网络同步管道极其舒服的负反馈层）
		CurrentVelocity = CurrentVelocity - (CurrentVelocity * StableFriction * DeltaSeconds);

		// 2. 注入毁灭性的加速度
		CurrentVelocity += MoveIntent * InfiniteAcceleration * DeltaSeconds;

		// 3. 严格限制最大速度
		float SpeedLimit = TargetMaxSpeed * SpeedScale;
		if (CurrentVelocity.SizeSquared() > FMath::Square(SpeedLimit))
		{
			CurrentVelocity = CurrentVelocity.GetSafeNormal() * SpeedLimit;
		}
	}
	else
	{
		// 【去急停惯性优化】无输入或处于死区时，摩擦力与制动爆发力双管齐下
		CurrentVelocity = CurrentVelocity - (CurrentVelocity * StableFriction * DeltaSeconds);

		FVector BrakingDrop = CurrentVelocity.GetSafeNormal() * InstantBraking * DeltaSeconds;
		if (BrakingDrop.SizeSquared() >= CurrentVelocity.SizeSquared())
		{
			CurrentVelocity = FVector::ZeroVector; // 瞬间切断，不准滑行
		}
		else
		{
			CurrentVelocity -= BrakingDrop;
		}
	}

	// 恢复垂直轴速度
	CurrentVelocity.Z = StrippedZVelocity;

	// 跳跃处理
	if (Inputs->bIsJumpPressed)
	{
		CurrentVelocity.Z = Settings->JumpImpulseForce / Settings->Mass;
	}

	OutProposedMove.LinearVelocity = CurrentVelocity;
	OutProposedMove.DirectionIntent = MoveIntent;

	if (CurrentVelocity.Size() > 0)
	{
		if (AActor* Owner = GetMoverComponent()->GetOwner())
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] DirectionIntent=%s LinearVelocity=%s"),
				   *UEnum::GetValueAsString(Owner->GetLocalRole()),
				   *MoveIntent.ToString(),
				   *CurrentVelocity.ToString()
			);
		}
	}
}

void ULrWalkMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{

	const ENetRole Role = GetMoverComponent()->GetOwner()->GetLocalRole();
	if (Role == ROLE_SimulatedProxy)
	{
		UE_LOG(LogTemp, Error,TEXT("SimulatedProxy Tick"));
	}

	
	FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
	const FMoverDefaultSyncState* StartingSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	if (!StartingSyncState) return;

	float DeltaTime = Params.TimeStep.StepMs * 0.001f;
	FVector ProposedVelocity = Params.ProposedMove.LinearVelocity;
	FVector DirectionIntent = Params.ProposedMove.DirectionIntent;

	FQuat CurrentRotation = StartingSyncState->GetOrientation_WorldSpace().Quaternion();
	FQuat TargetRotation = CurrentRotation;

	// 朝向瞬间同步
	if (!DirectionIntent.IsNearlyZero())
	{
		FRotator TargetRot = DirectionIntent.Rotation();
		TargetRot.Pitch = 0.0f;
		TargetRot.Roll = 0.0f;
		TargetRotation = TargetRot.Quaternion();
	}

	// 地面检索
	FFloorCheckResult FloorResult;
	UFloorQueryUtils::FindFloor(Params.MovingComps, 50.0f, 0.0f, true, Params.MovingComps.UpdatedComponent->GetComponentLocation(), FloorResult);
	if (FloorResult.bWalkableFloor && FloorResult.FloorDist < 2.f)
	{
       		ProposedVelocity.Z = 0.f;
	}
	if (!FloorResult.bWalkableFloor || (FloorResult.FloorDist > 10.0f))
	{
		OutputState.MovementEndState.NextModeName = LrAllModes::Air;
	}
	// 执行高精度物理移动（采用当帧最新的姿态进行物理扫掠，彻底封死因碰撞体角度不一致导致的误差）
	FHitResult Hit;
	Params.MovingComps.UpdatedComponent->MoveComponent(ProposedVelocity * DeltaTime, TargetRotation, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		const bool bIsWall = Hit.Normal.Z < 0.3f;
		if (bIsWall)
		{
			FMovementRecord MoveRecord;
			MoveRecord.SetDeltaSeconds(DeltaTime);
			UMovementUtils::TryMoveToSlideAlongSurface(Params.MovingComps, ProposedVelocity * DeltaTime, 1.0f - Hit.Time, TargetRotation, Hit.Normal, Hit, true, MoveRecord);
			ProposedVelocity = MoveRecord.GetRelevantVelocity();
		}
	}
	
	if (ProposedVelocity.Size() > 0)
	{
		if (AActor* Owner = GetMoverComponent()->GetOwner())
		{
			const FVector Location = OutputSyncState.GetLocation_WorldSpace();
			UE_LOG(LogTemp, Warning, TEXT("[%s] Loc=%s Vel=%s"),
			       *UEnum::GetValueAsString(Owner->GetLocalRole()),
			       *Location.ToString(),
			       *ProposedVelocity.ToString()
			);
		}
	}
	
	// 打包发送全网
	OutputSyncState.SetTransforms_WorldSpace(
		Params.MovingComps.UpdatedComponent->GetComponentLocation(),
		TargetRotation.Rotator(),
		ProposedVelocity,
		FVector::ZeroVector
	);
}

// void ULrWalkMovementMode::GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
// {
// 	// 获取基础同步状态和玩家当前步的输入
// 	const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
// 	const FLrMoverInputCmd* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();
//
// 	if (!SyncState || !Inputs) return;
//
// 	// 获取移动配置参数（优先从组件实例获取，其次从共享设置获取）
// 	const ULrMovementSettings* Settings = nullptr;
// 	if (const ULrMoverComponent* MyMover = Cast<ULrMoverComponent>(GetMoverComponent()))
// 	{
// 		Settings = MyMover->RealisticSettings;
// 	}
// 	if (!Settings)
// 	{
// 		Settings = GetMoverComponent()->FindSharedSettings<ULrMovementSettings>();
// 	}
// 	if (!Settings) return;
//
// 	// 基础变量准备
// 	float DeltaSeconds = TimeStep.StepMs * 0.001f;
// 	FVector CurrentVelocity = SyncState->GetVelocity_WorldSpace();
// 	
// 	// 步行模式只处理水平位移，暂存并清空 Z 轴速度
// 	float StrippedZVelocity = CurrentVelocity.Z;
// 	CurrentVelocity.Z = 0.0f;
//
// 	// 解析输入方向
// 	FVector MoveInput = Inputs->GetMoveInput();
// 	if (MoveInput.SizeSquared() < 0.01f) 
// 	{
// 		MoveInput = FVector::ZeroVector;
// 	}
// 	FVector MoveIntent = MoveInput.GetSafeNormal();
// 	bool bHasInput = !MoveIntent.IsNearlyZero();
//
// 	// ---------- 基础物理模拟：摩擦力与制动 ----------
// 	// 应用地面基础摩擦力
// 	if (Settings->GroundFriction > 0.0f)
// 	{
// 		CurrentVelocity = CurrentVelocity / (1.0f + (Settings->GroundFriction * DeltaSeconds));
// 	}
//
// 	// ---------- 基础物理模拟：加速与减速 ----------
// 	if (bHasInput)
// 	{
// 		// 有输入：向输入方向加速
// 		float TargetMaxSpeed = Settings->MaxWalkSpeed;
// 		
// 		// 状态修正速度（下蹲/冲刺）
// 		if (Inputs->bIsCrouchPressed)
// 		{
// 			TargetMaxSpeed *= Settings->CrouchSpeedMult;
// 		}
// 		else if (Inputs->bIsBlinkPressed)
// 		{
// 			float SprintMult = (Settings->SprintSpeedMult > 0.0f) ? Settings->SprintSpeedMult : 1.5f;
// 			TargetMaxSpeed *= SprintMult;
// 		}
//
// 		// 计算推力产生的加速度 (a = F / m)
// 		float PushForce = (Settings->MaxWalkForce < 100.0f) ? 192000.0f : Settings->MaxWalkForce;
// 		FVector Acceleration = (MoveIntent * PushForce) / Settings->Mass;
// 		
// 		// 叠加上步速度
// 		FVector NewVelocity = CurrentVelocity + (Acceleration * DeltaSeconds);
//
// 		// 软限制最大速度
// 		if (NewVelocity.SizeSquared() > FMath::Square(TargetMaxSpeed))
// 		{
// 			if (NewVelocity.SizeSquared() > CurrentVelocity.SizeSquared())
// 			{
// 				NewVelocity = NewVelocity.GetSafeNormal() * TargetMaxSpeed;
// 			}
// 		}
// 		CurrentVelocity = NewVelocity;
// 	}
// 	else
// 	{
// 		// 无输入：实施刹车制动
// 		float BrakeDecel = (Settings->BrakingDeceleration < 10.0f) ? 4000.0f : Settings->BrakingDeceleration;
// 		float CurrentSpeed = CurrentVelocity.Size();
// 		
// 		if (CurrentSpeed > 0.1f)
// 		{
// 			float NewSpeed = FMath::Max(0.0f, CurrentSpeed - (BrakeDecel * DeltaSeconds));
// 			CurrentVelocity = CurrentVelocity.GetSafeNormal() * NewSpeed;
// 		}
// 		else
// 		{
// 			CurrentVelocity = FVector::ZeroVector;
// 		}
// 	}
//
// 	// 还原 Z 轴速度分量（处理斜坡等物理自然下落/位移）
// 	CurrentVelocity.Z = StrippedZVelocity;
//
// 	// ---------- 跳跃处理 ----------
// 	// 注意：绝对不要在这里修改外部组件的任何 Bool 变量（例如 CacheMoverComponent->bJumpInitiated = true）
// 	// 那样会导致回滚时状态错乱。只把速度传出去即可。
// 	if (Inputs->bIsJumpPressed)
// 	{
// 		CurrentVelocity.Z = Settings->JumpImpulseForce / Settings->Mass;
// 	}
//
// 	// 填充提议移动结果
// 	OutProposedMove.LinearVelocity = CurrentVelocity;
// 	OutProposedMove.DirectionIntent = MoveIntent;
// }
//
// void ULrWalkMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
// {
// 	// 获取可变的输出同步状态引用
// 	FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
// 	const FMoverDefaultSyncState* StartingSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
// 	if (!StartingSyncState) return;
//
// 	float DeltaTime = Params.TimeStep.StepMs * 0.001f;
// 	FVector ProposedVelocity = Params.ProposedMove.LinearVelocity;
// 	FVector DirectionIntent = Params.ProposedMove.DirectionIntent;
// 	
// 	// 保持并处理平滑旋转
// 	FQuat CurrentRotation = StartingSyncState->GetOrientation_WorldSpace().Quaternion();
// 	FQuat TargetRotation = CurrentRotation;
//
// 	if (!DirectionIntent.IsNearlyZero())
// 	{
// 		FRotator TargetRot = DirectionIntent.Rotation();
// 		TargetRot.Pitch = 0.0f;
// 		TargetRot.Roll = 0.0f;
// 		FQuat DesiredRotation = TargetRot.Quaternion();
//
// 		// 确定性的单帧平滑旋转插值
// 		const float RotationSpeed = 240.0f; // 度/秒
// 		const float MaxRotationThisFrame = RotationSpeed * DeltaTime;
// 		const float AngleDiff = FMath::Acos(FMath::Clamp(CurrentRotation | DesiredRotation, -1.0f, 1.0f)) * (180.0f / PI);
//
// 		if (AngleDiff <= MaxRotationThisFrame || AngleDiff < 1.0f)
// 		{
// 			TargetRotation = DesiredRotation;
// 		}
// 		else
// 		{
// 			float T = FMath::Clamp(MaxRotationThisFrame / AngleDiff, 0.01f, 1.0f);
// 			TargetRotation = FQuat::Slerp(CurrentRotation, DesiredRotation, T);
// 			TargetRotation.Normalize();
// 		}
// 	}
//
// 	// ---------- 执行实际位移与障碍物滑动 ----------
// 	FHitResult Hit;
// 	// 仅移动位置，旋转在最后由 SyncState 统一更新，防止表现与物理冲突
// 	Params.MovingComps.UpdatedComponent->MoveComponent(ProposedVelocity * DeltaTime, CurrentRotation, true, &Hit);
//
// 	if (Hit.IsValidBlockingHit())
// 	{
// 		// 撞到障碍物，调用 Mover 库的滑动处理（爬坡、贴墙走）
// 		FMovementRecord MoveRecord;
// 		MoveRecord.SetDeltaSeconds(DeltaTime);
// 		UMovementUtils::TryMoveToSlideAlongSurface(Params.MovingComps, ProposedVelocity * DeltaTime, 1.0f - Hit.Time, CurrentRotation, Hit.Normal, Hit, true, MoveRecord);
// 		
// 		// 将实际滑动后的合法速度更新
// 		ProposedVelocity = MoveRecord.GetRelevantVelocity();
// 	}
//
// 	// ---------- 地面可行走检测与模式切换 ----------
// 	FFloorCheckResult FloorResult;
// 	// 50.0f 为最大向下扫掠检测距离
// 	UFloorQueryUtils::FindFloor(Params.MovingComps, 50.0f, 0.0f, true, Params.MovingComps.UpdatedComponent->GetComponentLocation(), FloorResult);
//
// 	// 如果失去悬浮/脚下空了，或者离地距离超过 10cm，自动切入空中模式
// 	bool bShouldGoToAir = !FloorResult.bWalkableFloor || (FloorResult.FloorDist > 10.0f);
// 	if (bShouldGoToAir)
// 	{
// 		OutputState.MovementEndState.NextModeName = LrAllModes::Air;
// 	}
//
// 	// ---------- 最终数据封包 ----------
// 	// 将最终计算出的确定坐标、旋转、速度塞入输出状态，Mover 插件会自动在多端将其平滑同步
// 	OutputSyncState.SetTransforms_WorldSpace(
// 		Params.MovingComps.UpdatedComponent->GetComponentLocation(),
// 		TargetRotation.Rotator(),
// 		ProposedVelocity,
// 		FVector::ZeroVector
// 	);
// }


/**
 * 获取角色脚下的表面摩擦力系数
 * 从 UpdatedComponent 位置向下发射 150 单位射线，查询物理材质的 Friction，
 * 并确保返回值不低于 0.1，以防完全无摩擦。如果没有命中或没有物理材质，返回默认值 1.0。
 * @param UpdatedComponent 需要检测的移动组件
 * @param World 当前世界指针，用于射线检测
 * @return 表面摩擦力系数（>= 0.1）
 */
float GetSurfaceFriction(const USceneComponent* UpdatedComponent, UWorld* World)
{
	if (!UpdatedComponent || !World) return 1.0f;
	// 射线起点为组件位置，终点向下150单位
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start - FVector(0, 0, 150.0f);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(UpdatedComponent->GetOwner()); // 忽略自身
	Params.bReturnPhysicalMaterial = true; // 返回物理材质

	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (Hit.PhysMaterial.IsValid())
		{
			return FMath::Max(Hit.PhysMaterial->Friction, 0.1f); // 确保最小摩擦力
		}
	}
	return 1.0f; // 默认摩擦系数为1
}


// /**
//  * 不改位置，
//  * 不改 Velocity，只输出「本帧希望往哪加速」
//  * @param StartState 
//  * @param TimeStep 
//  * @param OutProposedMove 
//  */
// void ULrWalkMovementMode::GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
// {
// 	// 获取同步状态（包含位置、速度、朝向等）
// 	const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
// 	// 获取当前帧的输入命令（移动方向、按键状态等）
// 	const FLrMoverInputCmd* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();
//
// 	// 尝试获取自定义的真实感移动设置（优先从 LrMoverComponent 中读取）
// 	const ULrMovementSettings* Settings = nullptr;
// 	if (const ULrMoverComponent* MyMover = Cast<ULrMoverComponent>(GetMoverComponent()))
// 	{
// 		Settings = MyMover->RealisticSettings;
// 	}
//
// 	// 如果没找到，则回退到共享设置
// 	if (!Settings) Settings = GetMoverComponent()->FindSharedSettings<ULrMovementSettings>();
//
// 	// 必要的指针检查，缺失任何一个都无法计算
// 	if (!SyncState || !Settings || !Inputs) return;
//
// 	// 将毫秒步长转换为秒
// 	float DeltaSeconds = TimeStep.StepMs * 0.001f;
// 	// 获取当前世界空间速度，并忽略垂直分量（步行只处理水平移动）
// 	FVector CurrentVelocity = SyncState->GetVelocity_WorldSpace();
// 	// double PreV = CurrentVelocity.Size();
// 	// if (GEngine)
// 	// {
// 	// 	FString DebugMsg = FString::Printf(TEXT("PreV: %.0f "), PreV);
// 	// 	GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Cyan, DebugMsg);
// 	// }
//
// 	CurrentVelocity.Z = CurrentVelocity.Z * 0.6f; //斜坡速度减速40%
//
// 	// 如果 SyncState 速度为 0 但上一帧有计算速度，使用上一帧的计算速度
// 	// 这避免了 Mover 插件同步状态延迟导致的速度为 0 问题
// 	if (CurrentVelocity.IsNearlyZero() && !LastCalculatedVelocity.IsNearlyZero())
// 	{
// 		CurrentVelocity = LastCalculatedVelocity;
// 	}
//
// 	// 获取原始输入向量
// 	FVector MoveInput = Inputs->GetMoveInput();
//
// 	// 输入死区（dead zone）：如果输入向量长度小于 0.1，则视为无输入
// 	if (MoveInput.Size() < 0.1f) MoveInput = FVector::ZeroVector;
//
// 	FVector MoveIntent = MoveInput.GetSafeNormal();
// 	// 计算脚下的表面摩擦力系数
// 	// float SurfaceFriction = GetSurfaceFriction(GetMoverComponent()->GetUpdatedComponent(), GetWorld());
// 	float SurfaceFriction = 1.f;
//
// 	float Speed = CurrentVelocity.Size(); // 上一帧结束时的速度,已经除去垂直度方向的速度
// 	bool bHasInput = !MoveIntent.IsNearlyZero(); // 是否有有效的输入方向
//
// 	// ---------- 计算相对于控制旋转（相机朝向）的前进/后退程度 ----------
// 	// ControlRotation 表示玩家控制器的朝向（通常为相机的朝向）
// 	FVector LookDir = Inputs->ControlRotation.Vector().GetSafeNormal2D();
// 	float ForwardDot = 0.0f;
// 	if (bHasInput)
// 	{
// 		// Dot Product：1.0 表示输入方向与相机朝向一致（前进），-1.0 表示完全相反（后退）
// 		ForwardDot = (MoveIntent | LookDir); // 1.0 = вперед, -1.0 = назад
// 	}
//
// 	// 判断是否正在移动（速度 > 1.0 认为有意义）
// 	bool bIsMoving = Speed > 1.0f;
// 	float Alignment = 0.0f; // 当前速度方向与输入方向的点积，用于判断同向/反向
// 	if (bIsMoving && bHasInput) Alignment = (CurrentVelocity.GetSafeNormal() | MoveIntent);
// 	bool bIsOpposing = bIsMoving && bHasInput && (Alignment < -0.1f); // 是否处于“反向移动”状态：正在移动，有输入，且方向几乎相反（点积 < -0.1）
//
// 	// ========== 加速惩罚逻辑（慢启动） ==========
// 	// 基于当前速度与输入方向的差异计算加速限制
// 	// 1. 从静止启动：速度为0，需要慢启动
// 	// 2. 180度转向：当前速度方向与输入方向相反，需要慢启动
// 	float SpeedAlignment = 0.0f;
// 	if (bIsMoving && bHasInput)
// 	{
// 		SpeedAlignment = (CurrentVelocity.GetSafeNormal() | MoveIntent);
// 	}
// 	else if (!bIsMoving && bHasInput)
// 	{
// 		// 从静止启动，视为最大惩罚
// 		SpeedAlignment = -1.0f;
// 	}
//
// 	// 计算加速系数：
// 	// - SpeedAlignment = 1.0（同向）-> 系数 = 1.0（满速）
// 	// - SpeedAlignment = 0.0（垂直）-> 系数 = 1.0 - Penalty * 0.5
// 	// - SpeedAlignment = -1.0（反向/静止）-> 系数 = 1.0 - Penalty（最慢）
// 	float AccelerationRamp = 1.0f;
// 	if (bHasInput)
// 	{
// 		float Penalty = FMath::Clamp(Settings->StartSpeedPenalty, 0.0f, 1.0f);
// 		// 将 [-1, 1] 映射到 [1-Penalty, 1]
// 		AccelerationRamp = 1.0f - Penalty * (1.0f - SpeedAlignment) * 0.5f;
// 	}
//
// 	// 每帧恢复加速进度（向 1.0 恢复）
// 	if (bHasInput && AccelerationRamp < 1.0f)
// 	{
// 		float RampUpRate = (Settings->AccelerationRampUpTime > 0.0f) ? (DeltaSeconds / Settings->AccelerationRampUpTime) : 1.0f;
// 		// 使用成员变量跟踪当前的实际加速进度
// 		CurrentAccelerationRamp = FMath::Min(1.0f, CurrentAccelerationRamp + RampUpRate);
// 		// 取两者中的较大值：当前恢复进度 或 基于方向的计算值
// 		AccelerationRamp = FMath::Max(AccelerationRamp, CurrentAccelerationRamp);
// 	}
// 	else if (!bHasInput)
// 	{
// 		// 无输入时重置加速进度
// 		CurrentAccelerationRamp = 0.0f;
// 	}
// 	else
// 	{
// 		// 有输入且不需要惩罚，保持满速
// 		CurrentAccelerationRamp = 1.0f;
// 	}
//
// 	// ========== 1. 摩擦处理 ==========
// 	// 简单的速度衰减：根据地面摩擦系数和表面摩擦力计算每帧衰减因子
// 	float FrictionForce = Settings->GroundFriction * SurfaceFriction;
// 	CurrentVelocity = CurrentVelocity / (1.0f + (FrictionForce * DeltaSeconds));
//
// 	// ========== 2. 制动（刹车）处理 ==========
// 	// 当没有输入时，施加制动力（不再对反向移动进行额外制动）
// 	if (!bHasInput)
// 	{
// 		// 获取制动减速度，如果设置值过小（小于10），则使用默认强制值 4000
// 		float BaseBrake = (Settings->BrakingDeceleration < 10.0f) ? 4000.0f : Settings->BrakingDeceleration;
// 		float BrakingMagnitude = BaseBrake;
//
// 		// 无输入时制动强度乘以5（更快速地停止）
// 		BrakingMagnitude *= 5.0f;
//
// 		// 表面摩擦也会影响制动力度
// 		float EffectiveFriction = FMath::Max(SurfaceFriction, 1.0f);
// 		BrakingMagnitude *= EffectiveFriction;
//
// 		// 应用制动，不使速度变为负值
// 		float NewSpeed = FMath::Max(0.0f, Speed - (BrakingMagnitude * DeltaSeconds));
//
// 		// 如果速度仍然显著，保留方向并缩小；否则置零
// 		if (Speed > 1.0f) CurrentVelocity = CurrentVelocity.GetSafeNormal() * NewSpeed;
// 		else CurrentVelocity = FVector::ZeroVector;
// 	}
//
// 	// ========== 3. 加速处理 ==========
// 	// 有输入时允许加速（包括180度转向时）
// 	bool bCanAccelerate = bHasInput;
//
// 	if (bCanAccelerate)
// 	{
// 		// 目标最大速度，基础值为步行最大速度
// 		float TargetMaxSpeed = Settings->MaxWalkSpeed;
// 		// 推进力，如果设置值太小则使用默认值 192000
// 		float PushForce = (Settings->MaxWalkForce < 100.0f) ? 192000.0f : Settings->MaxWalkForce;
//
// 		// ---------- 速度修正器（根据状态调整最大速度） ----------
// 		// 1. 下蹲：优先级最高，乘以蹲伏速度系数（例如 0.5）
// 		if (Inputs->bIsCrouchPressed)
// 		{
// 			TargetMaxSpeed *= Settings->CrouchSpeedMult; // 0.5 -> 减速到一半
// 		}
// 		// 2. 冲刺：仅在未下蹲时生效
// 		else if (Inputs->bIsBlinkPressed)
// 		{
// 			float SprintMult = (Settings->SprintSpeedMult > 0.0f) ? Settings->SprintSpeedMult : 1.5f;
// 			TargetMaxSpeed *= SprintMult;
// 		}
//
// 		// ---------- 推进力自动缩放（解决高摩擦下速度无法达到最大值的问题） ----------
// 		// 理论上需要的推力 = 质量 * 地面摩擦 * 目标速度
// 		float RequiredForce = Settings->Mass * Settings->GroundFriction * TargetMaxSpeed;
//
// 		// 如果配置的推力小于所需推力，则自动提升到所需值的 1.2 倍（留有 20% 余量）
// 		if (PushForce < RequiredForce)
// 		{
// 			PushForce = RequiredForce * 1.2f; // 
// 		}
// 		// -------------------------------------------------------------
// 		// 推力也受表面摩擦力影响（粗糙表面需要更大的力才能加速）
// 		PushForce *= SurfaceFriction;
//
// 		// 应用加速惩罚：将目标最大速度乘以加速系数
// 		float EffectiveMaxSpeed = TargetMaxSpeed * AccelerationRamp;
//
// 		// 根据牛顿第二定律计算加速度并叠加到当前速度
// 		FVector Acceleration = (MoveIntent * PushForce) / Settings->Mass;
// 		FVector NewVelocity = CurrentVelocity + (Acceleration * DeltaSeconds);
//
// 		// 软性速度限制（Soft Clamp）：
// 		// 仅当新速度向量大于目标速度，且大于当前速度时，才将其限制为目标速度。
// 		// 这样避免在转弯时不合理地截断速度分量。
// 		// 注意：使用 EffectiveMaxSpeed 而不是 TargetMaxSpeed 来应用惩罚
// 		if (NewVelocity.SizeSquared() > FMath::Square(EffectiveMaxSpeed))
// 		{
// 			if (NewVelocity.SizeSquared() > CurrentVelocity.SizeSquared())
// 			{
// 				NewVelocity = NewVelocity.GetSafeNormal() * EffectiveMaxSpeed;
// 			}
// 		}
// 		CurrentVelocity = NewVelocity;
// 	}
//
// 	// 最终：如果速度极小且无输入，直接归零，避免微小滑移
// 	if (CurrentVelocity.SizeSquared() < 1.0f && !bHasInput) CurrentVelocity = FVector::ZeroVector;
//
// 	// 填写提议移动：线速度为计算后的水平速度，方向意图为输入方向
// 	OutProposedMove.LinearVelocity = CurrentVelocity;
// 	OutProposedMove.DirectionIntent = MoveIntent;
//
// 	// 一段跳跃处理：如果按下跳跃键，在垂直方向施加一个瞬时速度，质量用配置的 JumpImpulseForce / Mass
// 	if (Inputs->bIsJumpPressed)
// 	{
// 		OutProposedMove.LinearVelocity.Z = Settings->JumpImpulseForce / Settings->Mass;
// 		if (CacheMoverComponent)
// 		{
// 			CacheMoverComponent->bJumpInitiated = true;
// 		}
// 	}
// 	else
// 	{
// 		CacheMoverComponent->bJumpInitiated = false;
// 	}
//
//
// 	// ---------- 调试信息：显示当前状态、速度与最大速度 ----------
// 	// if (GEngine)
// 	// {
// 	// 	FString MoveState = TEXT("Run");
// 	// 	if (Inputs->bIsBlinkPressed) MoveState = TEXT("SPRINT");
// 	// 	if (Inputs->bIsCrouchPressed) MoveState = TEXT("CROUCH");
// 	// 	if (ForwardDot < -0.5f) MoveState += TEXT(" (BACKWARDS)");
// 	//
// 	// 	// 显示同步状态速度（SyncSpeed）和计算后的目标速度（TargetSpeed）
// 	// 	FString DebugMsg = FString::Printf(TEXT("STATE: %s | SyncSpeed: %.0f | CalcSpeed: %.0f | Target: %.0f | Ramp: %.2f | Align: %.2f"),
// 	// 	                                   *MoveState,
// 	// 	                                   Speed,
// 	// 	                                   CurrentVelocity.Size(), //当前速度
// 	// 	                                   Settings->MaxWalkSpeed * AccelerationRamp,
// 	// 	                                   AccelerationRamp,
// 	// 	                                   SpeedAlignment);
// 	//
// 	// 	GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::Cyan, DebugMsg);
// 	// }
// 	// 保存当前计算的速度，用于下一帧
// 	LastCalculatedVelocity = CurrentVelocity; //提议速度
// }
//
//
// /**
//  * 模拟每一帧的物理移动，应用提议移动，处理碰撞、滑动与地面检测，
//  * 并决定是否切换到空中模式。
//  * @param Params 包含起始状态、时间步、提议移动等信息的参数
//  * @param OutputState 输出的移动结束数据，包含最终同步状态和可能的模式切换
//  */
// void ULrWalkMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
// {
// 	// 获取可变的输出同步状态引用
// 	FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
//
// 	// 获取起始同步状态（只读）
// 	const FMoverDefaultSyncState* StartingSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
// 	check(StartingSyncState);
//
// 	// 转换时间步长到秒
// 	float DeltaTime = Params.TimeStep.StepMs * 0.001f;
// 	// 提议移动中的线性速度（由 GenerateMove 计算）
// 	FVector ProposedVelocity = Params.ProposedMove.LinearVelocity;
// 	// 获取移动方向意图（在 GenerateMove 中设置）
// 	FVector DirectionIntent = Params.ProposedMove.DirectionIntent;
// 	// 当前朝向的四元数
// 	FQuat CurrentRotation = StartingSyncState->GetOrientation_WorldSpace().Quaternion();
//
// 	// ***************平滑旋转*************
// 	// 计算目标朝向：基于移动方向
// 	FQuat TargetRotation = CurrentRotation; // 默认保持当前朝向
// 	if (!DirectionIntent.IsNearlyZero())
// 	{
// 		// 根据移动方向计算目标旋转（只转 Yaw，不改变 Pitch/Roll）
// 		FRotator TargetRot = DirectionIntent.Rotation();
// 		TargetRot.Pitch = 0.f;
// 		TargetRot.Roll = 0.f;
// 		FQuat DesiredRotation = TargetRot.Quaternion();
//
// 		// 平滑插值旋转：使用指数衰减平滑让转向更自然
// 		// 旋转速度：240度/秒，降低旋转速度使动画更流畅
// 		const float RotationSpeed = 240.0f; // 度/秒
// 		const float MaxRotationThisFrame = RotationSpeed * DeltaTime;
// 		const float AngleDiff = FMath::Acos(FMath::Clamp(CurrentRotation | DesiredRotation, -1.0f, 1.0f)) * (180.0f / PI);
//
// 		if (AngleDiff <= MaxRotationThisFrame || AngleDiff < 1.0f)
// 		{
// 			// 角度差很小，直接设置到目标
// 			TargetRotation = DesiredRotation;
// 		}
// 		else
// 		{
// 			// 使用 Slerp 进行平滑插值，使用固定插值因子使旋转更稳定
// 			// 限制每帧最大旋转角度，避免跳跃
// 			const float T = FMath::Clamp(MaxRotationThisFrame / AngleDiff, 0.05f, 0.3f);
// 			TargetRotation = FQuat::Slerp(CurrentRotation, DesiredRotation, T);
// 			TargetRotation.Normalize();
// 		}
// 	}
//
// 	// ******************************** 正常地面移动 *********************************
// 	FHitResult Hit;
// 	// 只移动位置，不改变旋转，避免与 SyncState 的旋转冲突
// 	Params.MovingComps.UpdatedComponent->MoveComponent(ProposedVelocity * DeltaTime, CurrentRotation, true, &Hit);
//
// 	// 正常地面移动：先尝试按提议速度移动
// 	if (Hit.IsValidBlockingHit())
// 	{
// 		// 创建移动记录并利用工具函数尝试沿阻挡面滑动（处理爬坡、墙角等）
// 		FMovementRecord MoveRecord;
// 		MoveRecord.SetDeltaSeconds(DeltaTime);
// 		UMovementUtils::TryMoveToSlideAlongSurface(Params.MovingComps, ProposedVelocity * DeltaTime, 1.0f - Hit.Time, CurrentRotation, Hit.Normal, Hit, true, MoveRecord);
//
// 		// 更新速度为滑动后的实际速度,自动补偿z
// 		ProposedVelocity = MoveRecord.GetRelevantVelocity();
// 	}
//
// 	// 进行地面检测，确认脚下是否有可行走表面
// 	FFloorCheckResult FloorResult;
// 	UFloorQueryUtils::FindFloor(Params.MovingComps, 50.0f, 0.f, true, Params.MovingComps.UpdatedComponent->GetComponentLocation(), FloorResult);
//
// 	// 判断是否应该切换到空中模式
// 	bool bShouldGoAir = false;
// 	if (!FloorResult.bWalkableFloor || FloorResult.FloorDist > 10.0f) // 距离地面超过10cm则离地
// 	{
// 		bShouldGoAir = true;
// 	}
//
// 	// 根据离地状态设置下一模式
// 	if (bShouldGoAir)
// 	{
// 		OutputState.MovementEndState.NextModeName = LrAllModes::Air;
// 	}
//
// 	// 最终将更新后的位置、旋转和速度写入输出状态
// 	// 使用 TargetRotation 作为最终旋转，让 Mover 插件处理平滑
// 	OutputSyncState.SetTransforms_WorldSpace(Params.MovingComps.UpdatedComponent->GetComponentLocation(), TargetRotation.Rotator(), ProposedVelocity, FVector::ZeroVector);
//
// 	// ---------- 调试信息：显示当前状态、速度与最大速度 ----------
// 	// if (GEngine)
// 	// {
// 	// 	// 显示同步状态速度（SyncSpeed）和计算后的目标速度（TargetSpeed）
// 	// 	FString DebugMsg = FString::Printf(TEXT(" ProposedVelocity: %.0f "),
// 	// 	                                   ProposedVelocity.Size());
// 	// 	GEngine->AddOnScreenDebugMessage(3, 0.0f, FColor::Cyan, DebugMsg);
// 	// }
// }
