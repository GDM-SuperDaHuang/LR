#pragma once

#include "MovementMode.h"
#include "BaseMovementMode.generated.h"

UCLASS()
class UBaseMovementMode : public UMovementMode
{
    GENERATED_BODY()

public:
    static constexpr float MoveSpeed = 600.f;
    static constexpr float Gravity   = -980.f;

    /**
     * 1️⃣ ProduceInput        （生成输入 Command） 
     * 2️⃣ OnGenerateMove      （MovementMode 生成 ProposedMove）
     * 3️⃣ OnSimulationTick    （应用 Move → 推进 SyncState）
     * 
     * 先执行，意图生成（Intent），每帧触发， FMoverSimulationTickParams 的构建由 ProduceMoverInput 函数填充
     * ❌ OnGenerateMove 里不能改 SyncState
     * ❌ 不能做一次性逻辑（比如消耗 Jump）
     * 原因：
     * OnGenerateMove 可能被 回滚重复执行
     * 执行次数 ≠ 真实帧数
     * 
     * 
     * struct FMoverSimulationTickParams
     * {
        const FMoverSyncState& CurrentState;
        const FMoverInputCmdContext* InputCmdContext; // 仅 GenerateMove 用
        float StepMS;
     };
     */


     
    virtual void OnGenerateMove(const FMoverSimulationTickParams& Params, FMoverProposedMove& OutMove) override
    {
        const FMoverInputCmd* Input = static_cast<const FMoverInputCmd*>(Params.InputCmdContext);

        // 计算移动方向：从输入的 X 和 Y 分量构建向量，Z 为 0（平面移动）。
        // Dir 被限制在单位向量长度内，防止速度超过预期。
        FVector Dir(Input->MoveInput.X, Input->MoveInput.Y, 0.f);
        Dir = Dir.GetClampedToMaxSize(1.f);

        // 设置输出移动的线性速度：方向乘以移动速度，Z 分量保持当前状态（用于垂直速度，如跳跃）。
        // 填充到OnSimulationTick 的 FMoverSyncState& OutState
        OutMove.LinearVelocity = Dir * MoveSpeed;
        OutMove.LinearVelocity.Z = Params.CurrentState.LinearVelocity.Z;

        // ===== Mode 决策 =====
        //Params.CurrentState	上一帧 的最终 SyncState（只读），由 OnSimulationTick 上一帧的写入
        //Params.OutputSyncState ：意图,还不是还不是权威状态，可能被，被丢弃 / 回滚，	这一帧 要写入的 SyncState（可写）
        const FMoverSyncState& Sync = static_cast<const FMoverSyncState&>(Params.CurrentState);

        // 这里进行模式切换决策，但不实际修改状态（因为函数可能被回滚）。
        if (Input->bDashPressed && Sync.DashTimeRemaining <= 0.f)
        {
            Sync.DashDirection = Dir.IsNearlyZero() ? FVector::ForwardVector : Dir;
            Sync.DashTimeRemaining = 0.25f;
            Params.MoverComponent->QueueNextMode(TEXT("Dash"));
        }

        // 检查 Jump 输入：如果 Jump 按下且当前在地面上，则队列下一个模式为 "Jump"。
        if (Input->bJumpPressed && Sync.bIsOnGround)
        {
            Params.MoverComponent->QueueNextMode(TEXT("Jump"));
        }
    }

    //状态推进（State Advance）
    /**
     * 后执行
     * 改 FMoverSyncState、
     * 消耗一次性状态（Jump / Dash）
     * 位置积分
     * Mode 切换（QueueNextMode）
     * 只要会影响下一帧结果 → 必须在 OnSimulationTick
     */
    virtual void OnSimulationTick(const FMoverSimulationTickParams& Params, FMoverSyncState& OutState) override
    {
        // 这里的 Params.CurrentState 依旧是 OnSimulationTick的上一帧，和上面的OnGenerateMove的 Params.CurrentState是一致的
        FMoverSyncState& Sync = static_cast<FMoverSyncState&>(OutState);

        if (!Sync.bIsOnGround)
        {
            Sync.LinearVelocity.Z += Gravity * Params.StepMS * 0.001f;
        }

        Sync.Position += Sync.LinearVelocity * Params.StepMS * 0.001f;
    }
};

/***
 * Network Prediction 不是“帧驱动”，而是“时间驱动 + 回滚驱动”。Network Prediction的一帧代表一段时间
 * 固定步长子步（Substep）
 * 
→ 同一帧内：
   ├─ Step #1 → OnGenerateMove
   ├─ Step #2 → OnGenerateMove




之前一步，占用一帧
OnSimulationTick(
    Params.CurrentState = S(n-1),
    OutState = S(n) // (真实写入)
)


第一步，占用一帧
OnGenerateMove(
    Params.CurrentState = S(n),  (只读)==上一帧的OutState
    Params.OutputSyncState = Scratch(n+1), (临时草稿),可能被写,不保证生效，也不会传递到OnSimulationTick，只是参与多 Mode 竞争，因为Base / Dash / Jump 都可能 GenerateMove Network Prediction决策，预测失败整个推演可能被丢弃
    Params.InputCmdContext = InputCmd(n+1),
    OutMove
)

第二步，占用一帧
Network Prediction进行决策


第三步，占用一帧
OnSimulationTick(
    Params.CurrentState = S(n),   (只读)==上一帧的OutState
    OutState = S(n+1) // (真实写入)
)

...

第四步，占用一帧
OnGenerateMove(
    Params.CurrentState = S(n+1),  (只读)
    Params.OutputSyncState = Scratch(n+2), (临时草稿),可能被写,不保证生效
    Params.InputCmdContext = InputCmd(n+2),
    OutMove
)


*/



