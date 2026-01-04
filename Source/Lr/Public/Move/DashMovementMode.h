// DashMovementMode.h
#pragma once

#include "MovementMode.h"
#include "DashMovementMode.generated.h"

UCLASS()
class UDashMovementMode : public UMovementMode
{
    GENERATED_BODY()

public:
    static constexpr float DashSpeed    = 1200.f;
    static constexpr float DashDuration = 0.25f;

    virtual void OnGenerateMove(const FMoverSimulationTickParams& Params,FMoverProposedMove& OutMove) override
    {
        const FMoverSyncState& Sync = static_cast<const FMoverSyncState&>(Params.CurrentState);
        OutMove.LinearVelocity = Sync.DashDirection * DashSpeed;
    }

    virtual void OnSimulationTick(const FMoverSimulationTickParams& Params,FMoverSyncState& OutState) override
    {
        FMoverSyncState& Sync = static_cast<FMoverSyncState&>(OutState);

        Sync.DashTimeRemaining -= Params.StepMS * 0.001f;

        Sync.Position += Sync.LinearVelocity * Params.StepMS * 0.001f;

        if (Sync.DashTimeRemaining <= 0.f)
        {
            Params.MoverComponent->QueueNextMode(TEXT("Base"));
        }
    }
};
