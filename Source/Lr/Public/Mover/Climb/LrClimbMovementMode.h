// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "LrClimbMovementMode.generated.h"

class ULrMoverComponent;
/**
 * 
 */
UCLASS()
class LR_API ULrClimbMovementMode : public UBaseMovementMode
{
	GENERATED_BODY()

public:
	ULrClimbMovementMode();

	// 进入飞行模式时的初始化回调（可用于缓存 Mover 组件等）
	virtual void Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState) override;

	// 生成提议移动：根据输入计算本帧期望的飞行速度
	virtual void GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	// 模拟移动：根据提议速度实际移动组件，并输出同步状态
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;

	// 缓存所属的自定义 Mover 组件，避免每帧重复查找
	UPROPERTY()
	TObjectPtr<ULrMoverComponent> CacheMoverComponent;

private:
	// 当前墙面法线
	UPROPERTY()
	FVector WallNormal;


	// 墙面向上方向
	UPROPERTY()
	FVector WallUp;


	// 墙面右方向
	UPROPERTY()
	FVector WallRight;


	// 保存墙面距离
	float WallOffset = 45.f;


	// 重新检测墙
	bool FindClimbSurface(const FVector& Location, FHitResult& OutHit) const;
	
	// 是否允许攀爬
	bool CanClimbSurface(const FVector& Normal) const;
	
	// 更新墙坐标
	void UpdateWallBasis(const FVector& Normal);
};
