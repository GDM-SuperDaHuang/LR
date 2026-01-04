#pragma once

#include "MovementMode.h"
#include "JumpMovementMode.generated.h"

UCLASS()
class UJumpMovementMode : public UMovementMode
{
    GENERATED_BODY()

public:
    static constexpr float JumpZVelocity = 800.f;

    virtual void OnGenerateMove(const FMoverSimulationTickParams& Params,FMoverProposedMove& OutMove) override
    {
        OutMove.LinearVelocity = Params.CurrentState.LinearVelocity;
        OutMove.LinearVelocity.Z = JumpZVelocity;
    }

    virtual void OnSimulationTick(const FMoverSimulationTickParams& Params,FMoverSyncState& OutState) override{
        FMoverSyncState& Sync = static_cast<FMoverSyncState&>(OutState);

        if (!Sync.bJumpConsumed){
            // 只在第一次 Tick 执行
            Sync.LinearVelocity.Z = JumpZVelocity;
            Sync.bJumpConsumed = true;
            Sync.JumpCount++;
        }

        // Jump 是一次性注入 → 回 Base
        Params.MoverComponent->QueueNextMode(TEXT("Base"));
    }
};
