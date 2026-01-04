#pragma once

#include "CoreMinimal.h"
#include "MoverTypes.h"
#include "MoverInputCmd.generated.h"

/**
 * Network Prediction InputCmd
 * 只描述「这一帧玩家做了什么」
 */
USTRUCT()
struct FLrMoverInputCmd : public FMoverInputCmdContext
{
    GENERATED_BODY()

    UPROPERTY()
    FVector2D MoveInput = FVector2D::ZeroVector;

    uint8 bJumpPressed : 1;
    uint8 bDashPressed : 1;

    FMoverInputCmd()
        : bJumpPressed(false)
        , bDashPressed(false)
    {}
};

/**
 * Network Prediction 同步状态
 * ❗ 只要影响下一帧运动结果，就必须在这里
 */
USTRUCT()
struct FLrMoverSyncState : public FMoverSyncState
{
    GENERATED_BODY()

    // ===== 地面状态 =====
    UPROPERTY()
    bool bIsOnGround = true;

    // ===== Jump 状态 =====
    UPROPERTY()
    int32 JumpCount = 0;

    UPROPERTY()
    bool bJumpConsumed = false;

    // ===== Dash 状态 =====
    UPROPERTY()
    float DashTimeRemaining = 0.f;

    UPROPERTY()
    FVector DashDirection = FVector::ZeroVector;
};
