// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Walk/LrWalkMovementMode.h"

#include "MoveLibrary/FloorQueryUtils.h"
#include "MoveLibrary/MovementUtils.h"
#include "Mover/FRealisticMoverInputCmd.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/RealisticModes.h"
#include "Mover/RealisticMovementSettings.h"
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
	Params.AddIgnoredActor(UpdatedComponent->GetOwner());// 忽略自身
	Params.bReturnPhysicalMaterial = true;// 返回物理材质

	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (Hit.PhysMaterial.IsValid())
		{
			return FMath::Max(Hit.PhysMaterial->Friction, 0.1f);// 确保最小摩擦力
		}
	}
	return 1.0f;// 默认摩擦系数为1
}

/**
 * 不改位置，
 * 不改 Velocity，只输出「本帧希望往哪加速」
 * @param StartState 
 * @param TimeStep 
 * @param OutProposedMove 
 */
void ULrWalkMovementMode::GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
    // 获取同步状态（包含位置、速度、朝向等）
	const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
    // 获取当前帧的输入命令（移动方向、按键状态等）
    const FRealisticMoverInputCmd* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FRealisticMoverInputCmd>();
    
    // 尝试获取自定义的真实感移动设置（优先从 LrMoverComponent 中读取）
    const URealisticMovementSettings* Settings = nullptr;
    if (const ULrMoverComponent* MyMover = Cast<ULrMoverComponent>(GetMoverComponent()))
    {
        Settings = MyMover->RealisticSettings;
    }
    
    // 如果没找到，则回退到共享设置
    if (!Settings) Settings = GetMoverComponent()->FindSharedSettings<URealisticMovementSettings>();

    // 必要的指针检查，缺失任何一个都无法计算
    if (!SyncState || !Settings || !Inputs) return;

    // 将毫秒步长转换为秒
    float DeltaSeconds = TimeStep.StepMs * 0.001f;
    // 获取当前世界空间速度，并忽略垂直分量（步行只处理水平移动）
    FVector CurrentVelocity = SyncState->GetVelocity_WorldSpace();
    CurrentVelocity.Z = 0.f;

    // 获取原始输入向量
    FVector MoveInput = Inputs->GetMoveInput();
 
    // 输入死区（dead zone）：如果输入向量长度小于 0.1，则视为无输入
    if (MoveInput.Size() < 0.1f) MoveInput = FVector::ZeroVector;
 
    FVector MoveIntent = MoveInput.GetSafeNormal();
    // 计算脚下的表面摩擦力系数
    float SurfaceFriction = GetSurfaceFriction(GetMoverComponent()->GetUpdatedComponent(), GetWorld());
    float Speed = CurrentVelocity.Size();// 当前水平速率
    bool bHasInput = !MoveIntent.IsNearlyZero(); // 是否有有效的输入方向
 
    // ---------- 计算相对于控制旋转（相机朝向）的前进/后退程度 ----------
    // ControlRotation 表示玩家控制器的朝向（通常为相机的朝向）
    FVector LookDir = Inputs->ControlRotation.Vector().GetSafeNormal2D();
    float ForwardDot = 0.0f;
    if (bHasInput)
    {
        // Dot Product：1.0 表示输入方向与相机朝向一致（前进），-1.0 表示完全相反（后退）
        ForwardDot = (MoveIntent | LookDir); // 1.0 = вперед, -1.0 = назад
    }
 
    // ---------- 调试信息：显示当前状态、速度与最大速度 ----------
    if (GEngine)
    {
        FString MoveState = TEXT("Run");
        if (Inputs->bIsSprintPressed) MoveState = TEXT("SPRINT");
        if (Inputs->bIsCrouchPressed) MoveState = TEXT("CROUCH");
        if (ForwardDot < -0.5f) MoveState += TEXT(" (BACKWARDS)");
 
        FString DebugMsg = FString::Printf(TEXT("STATE: %s | Speed: %.0f / %.0f"),
            *MoveState,
            Speed,
            Settings->MaxWalkSpeed); // Показываем базовую макс скорость
 
        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Cyan, DebugMsg);
    }

    // 判断是否正在移动（速度 > 1.0 认为有意义）
    bool bIsMoving = Speed > 1.0f;
    float Alignment = 0.0f;    // 当前速度方向与输入方向的点积，用于判断同向/反向
    if (bIsMoving && bHasInput) Alignment = (CurrentVelocity.GetSafeNormal() | MoveIntent);
    bool bIsOpposing = bIsMoving && bHasInput && (Alignment < -0.1f);    // 是否处于“反向移动”状态：正在移动，有输入，且方向几乎相反（点积 < -0.1）

    // ========== 加速惩罚逻辑 ==========
    // 检测是否需要重置加速进度：
    // 1. 从静止启动（上一帧无输入，这一帧有输入）
    // 2. 大角度转向（输入方向与上一帧输入方向的点积 < 0.5，即角度 > 60度）
    bool bJustStarted = LastMoveIntent.IsNearlyZero() && bHasInput;
    float DirectionChangeDot = (LastMoveIntent.IsNearlyZero() || !bHasInput) ? 1.0f : (LastMoveIntent | MoveIntent);
    bool bSharpTurn = DirectionChangeDot < 0.5f; // 角度 > 60度视为大角度转向

    if (bJustStarted || bSharpTurn)
    {
        // 重置加速进度到惩罚状态
        CurrentAccelerationRamp = 1.0f - FMath::Clamp(Settings->StartSpeedPenalty, 0.0f, 1.0f);
    }

    // 每帧恢复加速进度
    if (CurrentAccelerationRamp < 1.0f && bHasInput)
    {
        float RampUpRate = (Settings->AccelerationRampUpTime > 0.0f) ? (DeltaSeconds / Settings->AccelerationRampUpTime) : 1.0f;
        CurrentAccelerationRamp = FMath::Min(1.0f, CurrentAccelerationRamp + RampUpRate);
    }
    else if (!bHasInput)
    {
        // 无输入时保持当前进度（下次启动时继续惩罚）
        // 或者可以重置：CurrentAccelerationRamp = 0.0f;
    }

    // 更新上一帧输入方向
    if (bHasInput)
    {
        LastMoveIntent = MoveIntent;
    }

 
    // ========== 1. 摩擦处理 ==========
    // 简单的速度衰减：根据地面摩擦系数和表面摩擦力计算每帧衰减因子
    float FrictionForce = Settings->GroundFriction * SurfaceFriction;
    CurrentVelocity = CurrentVelocity / (1.0f + (FrictionForce * DeltaSeconds));

    // ========== 2. 制动（刹车）处理 ==========
    // 当没有输入，或正在反向移动时，施加额外的制动力
    if (!bHasInput || bIsOpposing)
    {
        // 获取制动减速度，如果设置值过小（小于10），则使用默认强制值 4000
        float BaseBrake = (Settings->BrakingDeceleration < 10.0f) ? 4000.0f : Settings->BrakingDeceleration;
        float BrakingMagnitude = BaseBrake;

        // 反向移动时制动强度加倍；无输入时制动强度乘以5（更快速地停止）
        if (bIsOpposing) BrakingMagnitude *= 2.0f;
        else BrakingMagnitude *= 5.0f;

        // 表面摩擦也会影响制动力度
        float EffectiveFriction = FMath::Max(SurfaceFriction, 1.0f);
        BrakingMagnitude *= EffectiveFriction;

        // 应用制动，不使速度变为负值
        float NewSpeed = FMath::Max(0.0f, Speed - (BrakingMagnitude * DeltaSeconds));

        // 如果速度仍然显著，保留方向并缩小；否则置零
        if (Speed > 1.0f) CurrentVelocity = CurrentVelocity.GetSafeNormal() * NewSpeed;
        else CurrentVelocity = FVector::ZeroVector;
    }
 
    // ========== 3. 加速处理 ==========
    // 只有存在有效输入时才允许加速，但在高速反向时禁止加速（优先制动）
    bool bCanAccelerate = bHasInput;
    if (bIsOpposing && CurrentVelocity.Size() > 50.0f) bCanAccelerate = false;
 
    if (bCanAccelerate)
    {
        // 目标最大速度，基础值为步行最大速度
        float TargetMaxSpeed = Settings->MaxWalkSpeed;
        // 推进力，如果设置值太小则使用默认值 192000
        float PushForce = (Settings->MaxWalkForce < 100.0f) ? 192000.0f : Settings->MaxWalkForce;
 
        // ---------- 速度修正器（根据状态调整最大速度） ----------
        // 1. 下蹲：优先级最高，乘以蹲伏速度系数（例如 0.5）
        if (Inputs->bIsCrouchPressed)
        {
            TargetMaxSpeed *= Settings->CrouchSpeedMult; // 0.5 -> 减速到一半
        }
        // 2. 冲刺：仅在未下蹲时生效
        else if (Inputs->bIsSprintPressed)
        {
            float SprintMult = (Settings->SprintSpeedMult > 0.0f) ? Settings->SprintSpeedMult : 1.5f;
            TargetMaxSpeed *= SprintMult;
        }
 
        // 3. 后退移动惩罚：如果点积 < -0.5，说明几乎是纯后退，将最大速度降低 40%
        if (ForwardDot < -0.5f)
        {
            TargetMaxSpeed *= 0.6f; // Замедляем до 60%
        }
 
        // ---------- 推进力自动缩放（解决高摩擦下速度无法达到最大值的问题） ----------
        // 理论上需要的推力 = 质量 * 地面摩擦 * 目标速度
        float RequiredForce = Settings->Mass * Settings->GroundFriction * TargetMaxSpeed;
 
        // 如果配置的推力小于所需推力，则自动提升到所需值的 1.2 倍（留有 20% 余量）
        if (PushForce < RequiredForce)
        {
            PushForce = RequiredForce * 1.2f; // +20% запаса для ускорения
        }
        // -------------------------------------------------------------
        // 推力也受表面摩擦力影响（粗糙表面需要更大的力才能加速）
        PushForce *= SurfaceFriction;

        // 应用加速惩罚：将目标最大速度乘以当前加速进度
        float EffectiveMaxSpeed = TargetMaxSpeed * CurrentAccelerationRamp;

        // 根据牛顿第二定律计算加速度并叠加到当前速度
        FVector Acceleration = (MoveIntent * PushForce) / Settings->Mass;
        FVector NewVelocity = CurrentVelocity + (Acceleration * DeltaSeconds);
 
        // 软性速度限制（Soft Clamp）：
        // 仅当新速度向量大于目标速度，且大于当前速度时，才将其限制为目标速度。
        // 这样避免在转弯时不合理地截断速度分量。
        // 注意：使用 EffectiveMaxSpeed 而不是 TargetMaxSpeed 来应用惩罚
        if (NewVelocity.SizeSquared() > FMath::Square(EffectiveMaxSpeed))
        {
            if (NewVelocity.SizeSquared() > CurrentVelocity.SizeSquared())
            {
                NewVelocity = NewVelocity.GetSafeNormal() * EffectiveMaxSpeed;
            }
        }
        CurrentVelocity = NewVelocity;
    }

    // 最终：如果速度极小且无输入，直接归零，避免微小滑移
    if (CurrentVelocity.SizeSquared() < 1.0f && !bHasInput) CurrentVelocity = FVector::ZeroVector;

    // 填写提议移动：线速度为计算后的水平速度，方向意图为输入方向
    OutProposedMove.LinearVelocity = CurrentVelocity;
    OutProposedMove.DirectionIntent = MoveIntent;

    // 跳跃处理：如果按下跳跃键，在垂直方向施加一个瞬时速度，质量用配置的 JumpImpulseForce / Mass
    if (Inputs->bIsJumpPressed)
    {
        OutProposedMove.LinearVelocity.Z = Settings->JumpImpulseForce / Settings->Mass;
    }
}


/**
 * 模拟每一帧的物理移动，应用提议移动，处理碰撞、滑动与地面检测，
 * 并决定是否切换到空中模式。
 * @param Params 包含起始状态、时间步、提议移动等信息的参数
 * @param OutputState 输出的移动结束数据，包含最终同步状态和可能的模式切换
 */
void ULrWalkMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
    // 获取可变的输出同步状态引用
	FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
    
    // 获取起始同步状态（只读）
    const FMoverDefaultSyncState* StartingSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
    check(StartingSyncState);

    // 转换时间步长到秒
    float DeltaTime = Params.TimeStep.StepMs * 0.001f;
    // 提议移动中的线性速度（通常由 GenerateMove 计算）
    FVector ProposedVelocity = Params.ProposedMove.LinearVelocity;
    // 获取移动方向意图（在 GenerateMove 中设置）
    FVector DirectionIntent = Params.ProposedMove.DirectionIntent;
    // 当前朝向的四元数
    FQuat CurrentRotation = StartingSyncState->GetOrientation_WorldSpace().Quaternion();

    // 计算目标朝向：基于移动方向
    FQuat TargetRotation = CurrentRotation;  // 默认保持当前朝向
    if (!DirectionIntent.IsNearlyZero())
    {
        // 根据移动方向计算目标旋转（只转 Yaw，不改变 Pitch/Roll）
        FRotator TargetRot = DirectionIntent.Rotation();
        TargetRot.Pitch = 0.f;
        TargetRot.Roll = 0.f;
        FQuat DesiredRotation = TargetRot.Quaternion();

        // 平滑插值旋转：使用球面插值(Slerp)让转向更自然
        // 旋转速度：720度/秒，可根据需要调整
        const float RotationSpeed = 720.0f; // 度/秒
        const float MaxRotationThisFrame = RotationSpeed * DeltaTime;
        const float AngleDiff = FMath::Acos(FMath::Clamp(CurrentRotation | DesiredRotation, -1.0f, 1.0f)) * (180.0f / PI);

        if (AngleDiff <= MaxRotationThisFrame || AngleDiff < 1.0f)
        {
            // 角度差很小，直接设置到目标
            TargetRotation = DesiredRotation;
        }
        else
        {
            // 使用 Slerp 进行平滑插值
            const float T = MaxRotationThisFrame / AngleDiff;
            TargetRotation = FQuat::Slerp(CurrentRotation, DesiredRotation, T);
            TargetRotation.Normalize();
        }
    }

    // 如果垂直方向速度大于阈值（10.0），说明角色已经离地，应当立刻进入空中模式
    if (ProposedVelocity.Z > 10.0f)
    {
        // 设置下一个移动模式为空中模式
        OutputState.MovementEndState.NextModeName = RealisticModes::Air;
        FHitResult Hit;

        // 尝试移动组件，处理可能的碰撞
        Params.MovingComps.UpdatedComponent->MoveComponent(ProposedVelocity * DeltaTime, TargetRotation, true, &Hit);
        if (Hit.IsValidBlockingHit())// 如果碰撞到了物体，将速度投影到碰撞面以模拟滑行
        {
            FVector Slide = FVector::VectorPlaneProject(ProposedVelocity, Hit.Normal);
            ProposedVelocity = Slide;
        }

        // 以当前位置、旋转、处理后的速度更新输出同步状态
        OutputSyncState.SetTransforms_WorldSpace(Params.MovingComps.UpdatedComponent->GetComponentLocation(), TargetRotation.Rotator(), ProposedVelocity, FVector::ZeroVector);

        return;
    }

    FHitResult Hit;
    Params.MovingComps.UpdatedComponent->MoveComponent(ProposedVelocity * DeltaTime, TargetRotation, true, &Hit);

    // 正常地面移动：先尝试按提议速度移动
    if (Hit.IsValidBlockingHit())
    {
        // 创建移动记录并利用工具函数尝试沿阻挡面滑动（处理爬坡、墙角等）
        FMovementRecord MoveRecord;
        MoveRecord.SetDeltaSeconds(DeltaTime);
        UMovementUtils::TryMoveToSlideAlongSurface(Params.MovingComps, ProposedVelocity * DeltaTime, 1.0f - Hit.Time, TargetRotation, Hit.Normal, Hit, true, MoveRecord);

        // 更新速度为滑动后的实际速度
        ProposedVelocity = MoveRecord.GetRelevantVelocity();
    }

    // 进行地面检测，确认脚下是否有可行走表面
    FFloorCheckResult FloorResult;
    UFloorQueryUtils::FindFloor(Params.MovingComps, 50.0f, 0.71f, true, Params.MovingComps.UpdatedComponent->GetComponentLocation(), FloorResult);

    // 如果脚下有可行走的地面
    if (FloorResult.bWalkableFloor)
    {
        // 如果角色悬浮于地面之上（距离 > 0.1 单位），则向下移动以贴合地面
        if (FloorResult.FloorDist > 0.1f) Params.MovingComps.UpdatedComponent->MoveComponent(FVector(0, 0, -FloorResult.FloorDist), TargetRotation, true, nullptr);
    }
    else
    {
        // 没有可行走地面：设置下一模式为空中，意味着角色将开始下落
        OutputState.MovementEndState.NextModeName = RealisticModes::Air;
    }

    // 最终将更新后的位置、旋转和速度写入输出状态
    // 使用 TargetRotation 而不是当前组件旋转，确保角色朝向移动方向
    OutputSyncState.SetTransforms_WorldSpace(Params.MovingComps.UpdatedComponent->GetComponentLocation(), TargetRotation.Rotator(), ProposedVelocity, FVector::ZeroVector);

}
