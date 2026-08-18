// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "LrFlyMovementMode.generated.h"

class ULrMoverComponent;
/**
 * 飞行移动模式：处理角色不受重力约束的自由飞行
 * 输入方向直接映射为世界空间中的飞行速度，无地面检测与坠落逻辑
 */
UCLASS()
class LR_API ULrFlyMovementMode : public UBaseMovementMode
{
	GENERATED_BODY()

public:
	ULrFlyMovementMode();

	// 进入飞行模式时的初始化回调（可用于缓存 Mover 组件等）
	virtual void Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState) override;

	// 生成提议移动：根据输入计算本帧期望的飞行速度
	virtual void GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	// 模拟移动：根据提议速度实际移动组件，并输出同步状态
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;
	//相机影响
	UPROPERTY(EditAnywhere)
	bool IsPlayer = false; //是否是玩家，只有玩家有Camera。
	//----------------------------------------------------
	// 飞行参数（可在蓝图/细节面板中调整）
	//----------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fly")
	float MaxSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fly")
	float MaxVerticalSpeed = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fly")
	float Acceleration = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fly")
	float BrakingDeceleration = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fly")
	float StopThreshold = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fly")
	float TurnSpeed = 0.75f; //倾斜速度

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fly")
	float RollAngle = 60.f; //左右倾斜角度

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fly")
	float PitchAngle = 60.f; //上下倾斜角度
	
	// 缓存所属的自定义 Mover 组件，避免每帧重复查找
	UPROPERTY()
	TObjectPtr<ULrMoverComponent> CacheMoverComponent;

	// 飞机Roll倾斜插值状态（跨帧保持，确保平滑过渡）
	// UPROPERTY()
	// float RollInterpValue = 0.f;
	// // 飞机Roll倾斜插值状态（跨帧保持，确保平滑过渡）
	// UPROPERTY()
	// float PitchInterpValue = 0.f;
	
	UPROPERTY()
	float DeadZone = 0.2f;
};
