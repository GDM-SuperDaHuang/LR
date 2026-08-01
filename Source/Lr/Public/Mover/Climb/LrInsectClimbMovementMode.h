// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "LrInsectClimbMovementMode.generated.h"

class ULrMoverComponent;

/**
 * 昆虫攀爬移动模式
 * 局部坐标约定（贴合墙面时）：Z 轴 = WallNormal，指向墙外，也就是昆虫腹部朝向。
 * X 轴 = HeadDir，昆虫头部朝向（相机屏幕朝上投影到墙面），W/S 沿此轴移动。
 * 1. 贴近墙面：向前 40cm 射线找墙，命中后沿法线腹部吸附到墙，Activate 里实现。
 * 2. 在墙上：
 *    昆虫的中心，会向墙内发射射线检测墙的法线50cm，向墙的反方向发射射线200cm用与背部的墙用于起跳(向不实现todo)，
 *    昆虫的中心附近的上/下/左/右4个方向发射射线40cm，用于寻找上/下/左/右4个方向可能存在的墙，
 *    向墙内发射4根探空射线检测是否越过当前墙面边缘，探空未命中时，从射线末端朝角色中心方向（垂直于原法线）发折返射线，寻找新墙面。
 * 3. 离开墙后会切换到 Air 模式。
 * 4. 按角色局部坐标移动：A/D -> WallRight，W/S -> WallUp。
 * 5. 调试：2昆虫的中心射线+4方向射线+ 4 探空射线 + 4 折返射线 + 多个新墙面目标球体。
 * 6.现在已经完整操作映射UpdateWallOpBasis()，补充其他
 */
UCLASS()
class LR_API ULrInsectClimbMovementMode : public UBaseMovementMode
{
	GENERATED_BODY()

public:
	ULrInsectClimbMovementMode();

	virtual void Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState) override;
	virtual void GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;

	//墙面操作转换
	/**
	* 固定全局参考Up = (0, 0, 1)
	* 第一步：判断是否为水平面（天花板/地板）
	* 如果 WallNormal == (0, 0, 1) 或 WallNormal == (0, 0, -1)：
	* 水平面特判（防止叉乘结果为零向量）
	* Right = (1, 0, 0)
	* Forward = (0, -1, 0)
	* 
	* 否则：
	* 第二步：计算墙面的局部"右"和"前"（注意叉乘顺序！）
	* WallRight = normalize( Up × N )      // 保证水平横移
	* WallForward = normalize( N × Right ) // 保证沿坡面上下
	*  第三步：组装最终移动向量（关键公式）
	*  最终移动向量 = (input.x × Right) + (input.y × Forward) + (input.z × N)
	*/
	void UpdateWallBasis(const FVector& InWallNormal, const FVector& MoveInput);
private:
	FVector WallNormal = FVector::ZeroVector;
	FVector WallRight = FVector::ZeroVector;
	FVector WallFinalMove = FVector::ZeroVector;
	FVector WallForward = FVector::ZeroVector;

	//头部朝向（相机屏幕朝上投影到墙面），W/S 沿此方向移动
	FVector HeadDir = FVector::ZeroVector;

	//缓存的相机旋转，用于让昆虫头部朝向跟随相机
	FRotator CachedCameraRotation = FRotator::ZeroRotator;

	//墙面角色身体旋转转换
	void UpdateWallRotationBasis(const FVector& InWallNormal, FQuat CurrentRotation, FVector MoveInput);

	
	UPROPERTY()
	TObjectPtr<ULrMoverComponent> CacheMoverComponent;
	
	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float WallSearchDistance = 40.f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float StickDistance = 15.f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float ClimbSpeed = 400.f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float CenterRayLength = 50.f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float BackRayLength = 200.f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float DirectionRayLength = 40.f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float ProbeLength = 50.f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float EdgeReturnLength = 120.f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb|Debug")
	bool bDrawDebug = true;
};
