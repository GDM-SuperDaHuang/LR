// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Air/LrAirMovementMode.h"

#include "Mover/FLrMoverInputCmd.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMovementSettings.h"

ULrAirMovementMode::ULrAirMovementMode()
{
    // 构造函数：在创建此移动模式实例时，告知 Mover 系统需要加载 URealisticMovementSettings 类型的共享设置
	SharedSettingsClasses.Add(ULrMovementSettings::StaticClass());
}

// 生成提议移动：根据当前状态（速度、输入）计算出本帧的预期速度（不实际移动，仅计算）
void ULrAirMovementMode::GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
    // 获取当前的同步状态（包含速度、位置等）
	const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
    // 获取当前的输入命令（移动方向、按键状态等）
    const FLrMoverInputCmd* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();

    // 尝试从自定义的 Mover 组件中获取真实运动设置（优先使用组件自定义的实例）
    const ULrMovementSettings* Settings = nullptr;
    if (const ULrMoverComponent* MyMover = Cast<ULrMoverComponent>(GetMoverComponent()))
    {
        Settings = MyMover->RealisticSettings;
    }
    // 如果组件没有自定义设置，则回退到共享的全局设置
    if (!Settings)
    {
        Settings = GetMoverComponent()->FindSharedSettings<ULrMovementSettings>();
    }

    // 必要数据缺失则直接返回，不做移动
    if (!SyncState || !Settings) return;

    // 时间步长转换为秒
    float DeltaSeconds = TimeStep.StepMs * 0.001f;
    // 当前世界空间中的速度向量
    FVector CurrentVelocity = SyncState->GetVelocity_WorldSpace();

    // ========== 1. 计算重力加速度（加强重力） ==========
    // 获取世界重力加速度（通常是 -980 cm/s²）
    float WorldGravityZ = GetWorld()->GetGravityZ();
    // 应用重力缩放系数（例如 2.0），使角色下落更快、更“沉重”
    FVector GravityAccel = FVector(0, 0, WorldGravityZ * Settings->GravityScale);

    // 根据牛顿第二定律：力 = 质量 * 加速度
    FVector GravityForce = GravityAccel * Settings->Mass;

    // ========== 2. 计算空气阻力（模拟空气摩擦） ==========
    float Speed = CurrentVelocity.Size();
    FVector DragForce = FVector::ZeroVector;

    // 只有当速度足够大时才施加阻力，避免微小抖动
    if (Speed > 1.0f)
    {
        // 阻力大小 = 空气阻力系数 * 速度²（与真实空气阻力物理一致）
        float DragMagnitude = Settings->AirDragCoef * Speed * Speed;
        // 阻力方向与速度方向相反
        DragForce = -CurrentVelocity.GetSafeNormal() * DragMagnitude;
    }

    // ========== 3. 空中控制力（允许在空中轻微改变方向） ==========
    FVector InputForce = FVector::ZeroVector;
    if (Inputs)
    {
        // 获取水平移动输入（忽略 Z 轴，因为空中通常不能直接上下移动）
        FVector MoveInput = Inputs->GetMoveInput();
        MoveInput.Z = 0.f;
        // 空中控制力 = 输入方向 * 空中控制力强度（通常远小于地面加速度）
        InputForce = MoveInput * Settings->AirControlForce;
    }

    // 合力 = 重力 + 空气阻力 + 输入力
    FVector NetForce = GravityForce + DragForce + InputForce;
    FVector Acceleration = NetForce / Settings->Mass;// 加速度 = 合力 / 质量

    OutProposedMove.LinearVelocity = CurrentVelocity + (Acceleration * DeltaSeconds);// 更新速度：v = v₀ + a * Δt

    // ========== 终端速度限制 ==========
    // 限制下落最大速度，防止无限加速（模拟真实物理：空气阻力最终与重力平衡）
    // 这里限制为 -5300 cm/s ≈ -190 km/h，可根据游戏风格调整
    if (OutProposedMove.LinearVelocity.Z < -5300.0f)
    {
        OutProposedMove.LinearVelocity.Z = -5300.0f;
    }

    // 如果有输入，将输入方向作为意图方向（可用于后续动画或旋转）
    if (Inputs) OutProposedMove.DirectionIntent = Inputs->GetMoveInput();
}

// 模拟移动：根据提议移动实际移动组件，并处理碰撞和模式切换
void ULrAirMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
    // 获取输出同步状态（可变引用）
    FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
    // 获取起始同步状态（只读）
    const FMoverDefaultSyncState* StartingSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
    check(StartingSyncState);

    float DeltaTime = Params.TimeStep.StepMs * 0.001f;
    // 获取起始朝向（空中模式通常不自动旋转，所以保持原有朝向）
    FQuat CurrentRotation = StartingSyncState->GetOrientation_WorldSpace().Quaternion();

    // 尝试移动组件，使用提议线速度 * 时间步长
    FHitResult Hit;
    Params.MovingComps.UpdatedComponent->MoveComponent(Params.ProposedMove.LinearVelocity * DeltaTime, CurrentRotation, true, &Hit);

    // 处理碰撞
    if (Hit.IsValidBlockingHit())
    {
        // 如果碰撞点的法线向上分量 >= 0.71（约 45° 以内），判定为“地面”
        // 0.71 是 cos(45°)，表示相对平坦的表面
        if (Hit.Normal.Z >= 0.7f)//(Hit.Normal.Z >= 0.71f)
        {
            // 触地 -> 切换到行走模式
            OutputState.MovementEndState.NextModeName = LrAllModes::Walk;
        }
        else
        {
            // 撞到墙壁或斜坡：将剩余的速度投影到碰撞面的切平面，实现“滑墙”效果
            // 1.0f - Hit.Time 表示剩余的移动时间比例
            FVector SlideDelta = FVector::VectorPlaneProject(Params.ProposedMove.LinearVelocity * DeltaTime, Hit.Normal) * (1.0f - Hit.Time);
            // 执行滑动移动
            Params.MovingComps.UpdatedComponent->MoveComponent(SlideDelta, CurrentRotation, true, nullptr);
        }
    }

    // 最终更新输出状态：位置、旋转、速度（与提议速度一致，因为碰撞已经处理完毕）
     OutputSyncState.SetTransforms_WorldSpace(
        Params.MovingComps.UpdatedComponent->GetComponentLocation(),
        Params.MovingComps.UpdatedComponent->GetComponentRotation(),
        Params.ProposedMove.LinearVelocity,
        FVector::ZeroVector);
}
