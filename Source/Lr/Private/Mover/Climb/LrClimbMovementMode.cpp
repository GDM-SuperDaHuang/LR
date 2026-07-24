// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/Climb/LrClimbMovementMode.h"

#include "MoverComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Mover/FLrMoverInputCmd.h"
#include "Mover/LrAllModes.h"

/**
 * 攀爬移动模式
 *
 * 该模式允许角色在可攀爬墙面上自由移动（上下/左右），
 * 并在失去墙面支持时自动切换到下落模式。
 */

ULrClimbMovementMode::ULrClimbMovementMode()
{
}

/**
 * 进入攀爬模式时调用
 *
 * 尝试在当前位置寻找可攀爬墙面，若命中则初始化墙面坐标系。
 */
void ULrClimbMovementMode::Activate(const FMoverEventContext& Context, FName PrevModeName, const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, FMoverSyncState* OutSyncState, FMoverAuxStateContext* OutAuxState)
{
	Super::Activate(Context, PrevModeName, SimContext, StartState, OutSyncState, OutAuxState);
	const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();

	// 没有有效的同步状态时直接返回，无法初始化位置信息
	if (!SyncState)
	{
		return;
	}

	// 在当前位置做一次墙面检测，用于初始化墙面法线
	FHitResult Hit;
	if (FindClimbSurface(SyncState->GetLocation_WorldSpace(), Hit))
	{
		WallNormal = Hit.Normal;
		UpdateWallBasis(WallNormal);
	}
}

/**
 * 根据输入生成本帧提议移动
 *
 * 将 WASD 输入映射到墙面的局部坐标系：
 * - A/D：沿墙面左右移动（WallRight）
 * - W/S：沿墙面上/下移动（WallUp）
 */
void ULrClimbMovementMode::GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	Super::GenerateMove_Implementation(SimContext, StartState, TimeStep, OutProposedMove);
	const FLrMoverInputCmd* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FLrMoverInputCmd>();
	if (!Inputs)
	{
		return;
	}

	FVector Input = Inputs->GetMoveInput();
	const float Speed = 350.f;
	FVector Velocity = FVector::ZeroVector;

	// A / D：沿墙面左右移动
	Velocity += WallRight * Input.X * Speed;
	// W / S：沿墙面上/下移动
	Velocity += WallUp * Input.Y * Speed;

	OutProposedMove.LinearVelocity = Velocity;
	OutProposedMove.DirectionIntent = Velocity;
}

/**
 * 每帧模拟攀爬物理
 *
 * 流程：
 * 1. 检测前方是否有可攀爬墙面；
 * 2. 更新墙面局部坐标系；
 * 3. 根据提议速度计算位移；
 * 4. 修正角色与墙面的距离，使其保持贴墙；
 * 5. 移动角色并朝向墙面；
 * 6. 输出新的世界空间变换。
 */
void ULrClimbMovementMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
	Super::SimulationTick_Implementation(Params, OutputState);
	FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
	TWeakObjectPtr<USceneComponent> UpdatedComp = Params.MovingComps.UpdatedComponent;
	if (!UpdatedComp.Get())
	{
		return;
	}

	// 将毫秒时间步转换为秒
	const float DeltaTime = Params.TimeStep.StepMs * 0.001f;

	// 当前组件世界位置
	FVector Location = UpdatedComp->GetComponentLocation();

	//----------------------------------
	// 1. 检测墙面
	//----------------------------------
	FHitResult Hit;
	if (!FindClimbSurface(Location, Hit))
	{
		// 失去墙面支持，切换到下坠（Air）模式
		OutputState.MovementEndState.NextModeName = LrAllModes::Air;
		return;
	}

	// 根据命中法线重新计算墙面局部坐标系
	UpdateWallBasis(Hit.Normal);

#if ENABLE_DRAW_DEBUG
	// 绘制墙面局部坐标系：红色=墙面法线，绿色=墙上方向，蓝色=墙右方向
	const FVector BasisOrigin = UpdatedComp->GetComponentLocation() + FVector::UpVector * 50.f;
	const float AxisLen = 40.f;
	DrawDebugDirectionalArrow(GetWorld(), BasisOrigin, BasisOrigin + WallNormal * AxisLen, 12.f, FColor::Red, false, -1.f, 0, 1.5f);
	DrawDebugDirectionalArrow(GetWorld(), BasisOrigin, BasisOrigin + WallUp * AxisLen, 12.f, FColor::Green, false, -1.f, 0, 1.5f);
	DrawDebugDirectionalArrow(GetWorld(), BasisOrigin, BasisOrigin + WallRight * AxisLen, 12.f, FColor::Blue, false, -1.f, 0, 1.5f);
#endif

	//----------------------------------
	// 2. 根据输入速度计算基础位移
	//----------------------------------
	FVector MoveDelta = Params.ProposedMove.LinearVelocity * DeltaTime;

	//----------------------------------
	// 3. 保持贴墙距离
	//----------------------------------
	// 当前命中距离与期望的墙面偏移量之间的差值，沿法线方向拉回
	float Distance = Hit.Distance;
	FVector Correction = -WallNormal * (Distance - WallOffset);
	MoveDelta += Correction;

	//----------------------------------
	// 4. 计算朝向墙面的旋转
	//----------------------------------
	FVector Forward = -WallNormal;
	FRotator Rotation = Forward.Rotation();
	FHitResult MoveHit;

	// 应用移动与旋转
	UpdatedComp->MoveComponent(
		MoveDelta,
		Rotation.Quaternion(),
		true,
		&MoveHit
	);

	//----------------------------------
	// 5. 输出最终变换到同步状态
	//----------------------------------
	OutputSyncState.SetTransforms_WorldSpace(
		UpdatedComp->GetComponentLocation(),
		Rotation,
		Params.ProposedMove.LinearVelocity,
		FVector::ZeroVector
	);
}

/**
 * 沿墙面法线方向做射线检测，寻找可攀爬表面
 *
 * @param Location  射线起点（角色位置）
 * @param OutHit    命中结果
 * @return true 表示命中且法线满足攀爬条件
 */
bool ULrClimbMovementMode::FindClimbSurface(const FVector& Location, FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// 墙面法线尚未初始化时（首次进入攀爬模式），使用角色朝向作为搜索方向，
	// 避免 WallNormal 为零导致射线长度为零、永远检测不到墙面。
	FVector SearchDir;
	if (!WallNormal.IsNearlyZero())
	{
		// WallNormal 指向角色外侧，反向即为朝向墙面内部
		SearchDir = -WallNormal;
	}
	else
	{
		AActor* Owner = GetMoverComponent() ? GetMoverComponent()->GetOwner() : nullptr;
		SearchDir = Owner ? Owner->GetActorForwardVector() : FVector::ForwardVector;
	}

	// 沿搜索方向发射射线，最大检测距离 80cm
	FVector Start = Location;
	FVector End = Start + SearchDir * 80.f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetMoverComponent()->GetOwner());
	bool bHit = World->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_WorldStatic,
		Params
	);

#if ENABLE_DRAW_DEBUG
	const bool bCanClimb = bHit && CanClimbSurface(OutHit.Normal);
	DrawDebugLine(World, Start, End, bCanClimb ? FColor::Green : FColor::Red, false, -1.f, 0, 2.f);
	if (bHit)
	{
		DrawDebugSphere(World, OutHit.ImpactPoint, 8.f, 12, bCanClimb ? FColor::Green : FColor::Orange, false, -1.f, 0, 1.f);
		DrawDebugDirectionalArrow(World, OutHit.ImpactPoint, OutHit.ImpactPoint + OutHit.Normal * 30.f, 15.f, FColor::Yellow, false, -1.f, 0, 1.f);
	}
#endif

	return bHit && CanClimbSurface(OutHit.Normal);
}

/**
 * 判断给定法线是否允许攀爬
 *
 * 过于水平的表面（接近地面/天花板）不适合攀爬，
 * 这里通过法线与世界上方向量的点积过滤掉过陡的表面。
 */
bool ULrClimbMovementMode::CanClimbSurface(const FVector& Normal) const
{
	float Dot = FVector::DotProduct(Normal, FVector::UpVector);
	// 太接近地面（法线朝上）时禁止攀爬
	if (Dot > 0.75f)
	{
		return false;
	}
	return true;
}

/**
 * 根据墙面法线更新墙面局部坐标系
 *
 * WallNormal：墙面法线（指向角色外侧）
 * WallUp：世界 Up 投影到墙面切平面后的方向
 * WallRight：WallUp 与 WallNormal 的叉积，构成右手坐标系
 */
void ULrClimbMovementMode::UpdateWallBasis(const FVector& Normal)
{
	WallNormal = Normal.GetSafeNormal();
	// 将世界 Up 向量投影到墙面切平面，得到墙面上的“上”方向
	WallUp = FVector::VectorPlaneProject(FVector::UpVector, WallNormal).GetSafeNormal();
	WallRight = FVector::CrossProduct(WallUp, WallNormal).GetSafeNormal();
}
