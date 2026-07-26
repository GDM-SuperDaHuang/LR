// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "LrClimbMovementMode.generated.h"

class ULrMoverComponent;

/**
 * 昆虫攀爬移动模式
 * 局部坐标约定（贴合墙面时）：Z 轴 = WallNormal，指向墙外，也就是昆虫腹部朝向。
 * X 轴 = WallRight（Tangent），昆虫左右方向，A/D 沿此轴移动。
 * Y 轴 = WallUp（Bitangent），昆虫前后/上下方向，W/S 沿此轴移动。
 * 1. 贴近墙面：向前 80 cm 射线找墙，命中后沿法线吸附到墙，Activate 里实现。
 * 2. 在墙上：中心射线保持腹部贴墙。上/下/左/右4个位置，向墙内发射4根探空射线检测是否越过当前墙面边缘，
 *    探空未命中时，从射线末端朝角色中心方向（垂直于原法线）发折返射线，寻找新墙面。
 * 3. 离开墙后会切换到 Air 模式。
 * 4. 按角色局部坐标移动：A/D -> WallRight，W/S -> WallUp。
 * 5. 调试：向前 80 cm 射线 + 中心射线 + 4 探空射线 + 4 折返射线 + 新墙面目标球体。
 */
UCLASS()
class LR_API ULrClimbMovementMode : public UBaseMovementMode
{
	GENERATED_BODY()

public:
	ULrClimbMovementMode();

	virtual void Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState) override;
	virtual void GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;

private:
	void UpdateWallBasis(const FVector& InNormal);

	UPROPERTY()
	TObjectPtr<ULrMoverComponent> CacheMoverComponent;

	// 墙面局部坐标系（世界空间）
	FVector WallNormal = FVector::ZeroVector;
	FVector WallRight = FVector::ZeroVector;
	FVector WallUp = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Climb")
	float WallSearchDistance = 80.f;

	UPROPERTY(EditAnywhere, Category = "Climb")
	float StickDistance = 15.f;

	UPROPERTY(EditAnywhere, Category = "Climb")
	float ClimbSpeed = 400.f;

	UPROPERTY(EditAnywhere, Category = "Climb")
	float ProbeOffset = 50.f;

	UPROPERTY(EditAnywhere, Category = "Climb")
	float ProbeLength = 50.f;

	UPROPERTY(EditAnywhere, Category = "Climb")
	float BackRayLength = 120.f;

	UPROPERTY(EditAnywhere, Category = "Climb|Debug")
	bool bDrawDebug = true;
};
