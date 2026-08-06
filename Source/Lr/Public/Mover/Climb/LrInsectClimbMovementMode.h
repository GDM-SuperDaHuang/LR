// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "LrInsectClimbMovementMode.generated.h"

class ULrMoverComponent;

/**
 * 昆虫攀爬移动模式
 * 局部坐标约定（贴合墙面时）：Z 轴 = WallNormal，指向墙外，也就是昆虫腹部朝向。
 * X 轴 = HeadDir，昆虫头部朝向（墙面基向量），W/S 沿此轴移动。
 * 1. 贴近墙面：向前 40cm 射线找墙，命中后沿法线腹部吸附到墙，Activate 里实现。
 * 2. 在墙上：
 *    昆虫的中心，会向墙内发射射线检测墙的法线50cm，向墙的反方向发射射线200cm用与背部的墙用于起跳(向不实现todo)，
 *    昆虫的中心附近的上/下/左/右4个方向发射射线40cm，用于寻找上/下/左/右4个方向可能存在的墙，
 *    向墙内发射4根探空射线检测是否越过当前墙面边缘，探空未命中时，从射线末端朝角色中心方向（垂直于原法线）发折返射线，寻找新墙面。
 * 3. 离开墙后会切换到 Air 模式。
 * 4. 按角色局部坐标移动：A/D -> WallRight(水平绕圈)，W/S -> WallUp(上下绕圈)。
 * 5. 调试：2昆虫的中心射线+4方向射线+ 4 探空射线 + 4 折返射线 + 多个新墙面目标球体。
 * 6.现在已经完整操作映射UpdateWallOpBasis()，补充其他
 */

//墙信息
USTRUCT()
struct FLrWallInfo
{
	GENERATED_BODY()

public:
	FVector WallNormal = FVector::ZeroVector; //法线
	FVector WallImpactPoint = FVector::ZeroVector; //击中点
};


UCLASS()
class LR_API ULrInsectClimbMovementMode : public UBaseMovementMode
{
	GENERATED_BODY()

public:
	virtual void Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState) override;
	virtual void GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;

private:
	FVector WallNormal = FVector::ZeroVector;
	FVector WallRight = FVector::ZeroVector;
	FVector WallFinalMove = FVector::ZeroVector;
	FVector WallForward = FVector::ZeroVector;
	//头部朝向（墙面基向量），W/S 沿此方向移动
	FVector HeadDir = FVector::ZeroVector;

	//墙面操作转换
	/**
	* W/S 方向（WallForward）优先沿用头部朝向 HeadDir 在墙面切平面上的投影，
	* 使按住 W/S 能沿原方向连续绕过墙角（与 A/D 水平绕圈对称）。
	* 翻越顶/底面时，BeginWallTransition 用过渡滑动方向预先重置 HeadDir。
	* 仅当 HeadDir 与墙面法线平行（首次激活 / 未绕角落上水平面）时才退回全局参考：
	* 如果 WallNormal == (0, 0, 1) 或 WallNormal == (0, 0, -1)：
	* 水平面特判（防止叉乘结果为零向量）Right = (1, 0, 0)，Forward = (0, -1, 0)
	* 否则：WallRight = normalize( Up × N )，WallForward = normalize( N × Right )
	* 组装最终移动向量：WallFinalMove = (input.x × WallRight) + (input.y × WallForward) + (input.z × N)
	*/
	void UpdateWallBasis(const FVector& InWallNormal, const FVector& MoveInput);
	//墙面角色身体旋转转换
	void UpdateWallRotationBasis(FQuat CurrentRotation, FVector MoveInput);

	//绕角过渡：从旧墙平滑滑到新墙（位置+旋转插值，替代瞬移）
	void BeginWallTransition(const FVector& NewNormal, const FVector& ImpactPoint);

	//寻找分数最高的墙
	int32 FindBestScoreWall(TArray<FLrWallInfo>& LrNewWallInfos, const FVector& CurPos);

	//绕角过渡状态
	bool bWallTransition = false;
	FVector TransitionTargetNormal = FVector::ZeroVector;
	FVector TransitionTargetPos = FVector::ZeroVector;
	FQuat TransitionTargetRot = FQuat::Identity;
	FVector TransitionStartPos = FVector::ZeroVector;
	FQuat TransitionStartRot = FQuat::Identity;
	float TransitionAlpha = 0.f;
	

	UPROPERTY()
	TObjectPtr<ULrMoverComponent> CacheMoverComponent;

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float WallSearchDistance = 40.f; //地面接触墙

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float StickDistance = 15.f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float ClimbSpeed = 400.f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb|Transition")
	float WallTransitionTime = 0.25f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float CenterRayLength = 50.f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float BackRayLength = 200.f;

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float DirectionRayLength = 40.f; //四方向射线长度

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float ProbeLength = 30.f; //探空长度（纵深长度）

	UPROPERTY(EditAnywhere, Category = "InsectClimb")
	float EdgeReturnLength = 120.f; //折返射线

	UPROPERTY(EditAnywhere, Category = "InsectClimb|Debug")
	bool bDrawDebug = true;
};
