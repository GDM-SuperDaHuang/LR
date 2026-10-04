// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/GC/LrDestructionComponent.h"

#include "GeometryCollection/GeometryCollectionComponent.h"

// Sets default values for this component's properties
ULrDestructionComponent::ULrDestructionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	DefaultImpactParams.HitPoint = FVector::ZeroVector;
	DefaultImpactParams.ImpactDirection = FVector::ForwardVector;

	DefaultImpactParams.Strain = 100.0f;
	DefaultImpactParams.BreakRadius = 100.0f;

	DefaultImpactParams.PropagationDepth = 1;
	DefaultImpactParams.PropagationFactor = 0.5f;

	DefaultImpactParams.ForwardSpeed = 1200.0f;
	DefaultImpactParams.RadialSpeed = 250.0f;
	DefaultImpactParams.RandomSpeed = 100.0f;

	DefaultImpactParams.bUseRadialSpread = true;
	DefaultImpactParams.bUseRandomSpread = true;

	DefaultImpactParams.RandomSeed = 1337;
	// ...
}


void ULrDestructionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoFindGeometryCollection && !GeometryCollection)
	{
		ResolveGeometryCollection();
	}
}

bool ULrDestructionComponent::ApplyImpact(const FLrDestructionImpactParams& Params)
{
	if (!IsValidGeometryCollection())
	{
		UE_LOG(LogTemp, Warning, TEXT("LrDestruction: GeometryCollection is invalid."));
		return false;
	}

	if (!bAllowRepeatedImpact && bHasImpacted)
	{
		return false;
	}

	if (Params.BreakRadius <= 0.0f)
	{
		return false;
	}

	if (Params.Strain <= 0.0f)
	{
		return false;
	}

	const FVector ImpactDirection = Params.ImpactDirection.GetSafeNormal();

	if (ImpactDirection.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning, TEXT("LrDestruction: ImpactDirection is zero."));
		return false;
	}

	/**
	 * 这里没有直接使用 ItemIndex。
	 *
	 * 因为对于通用 ApplyImpact，
	 * 我们只有一个世界空间 HitPoint。
	 *
	 * ApplyExternalStrain 的 ItemIndex
	 * 对于碰撞命中可以来自 HitResult。
	 *
	 * 如果没有 ItemIndex，
	 * 这里使用 -1，让 Geometry Collection
	 * 根据 Location + Radius 进行场影响。
	 */
	const int32 ItemIndex = INDEX_NONE;

	/**
	 * 第一阶段：
	 *
	 * HitPoint + Radius + Strain
	 *
	 * 让撞击点附近的 Cluster 发生破坏。
	 */
	if (!ApplyStrain(ItemIndex, Params))
	{
		return false;
	}

	/**
	 * 第二阶段：
	 *
	 * 给发生破坏的 Cluster 设置
	 * Breaking Linear Velocity。
	 *
	 * 这里的关键就是：
	 * ImpactDirection * ForwardSpeed
	 *
	 * 让碎片整体朝子弹方向飞。
	 */
	ApplyBreakingVelocity(ItemIndex, Params);

	bHasImpacted = true;

	return true;
}

bool ULrDestructionComponent::ApplyImpactSimple(const FVector& HitPoint, const FVector& ImpactDirection, float ForwardSpeed, float BreakRadius)
{
	FLrDestructionImpactParams Params = DefaultImpactParams;

	Params.HitPoint = HitPoint;
	Params.ImpactDirection = ImpactDirection.GetSafeNormal();
	Params.ForwardSpeed = ForwardSpeed;
	Params.BreakRadius = BreakRadius;

	return ApplyImpact(Params);
}

bool ULrDestructionComponent::ApplyBulletImpact(const FHitResult& Hit, const FVector& BulletVelocity, float Damage)
{
	if (!IsValidGeometryCollection())
	{
		return false;
	}

	const FVector Direction = BulletVelocity.IsNearlyZero() ? -Hit.ImpactNormal : BulletVelocity.GetSafeNormal();

	FLrDestructionImpactParams Params = DefaultImpactParams;

	Params.HitPoint = Hit.ImpactPoint;

	Params.ImpactDirection = Direction;

	/**
	 * 可以简单把 Damage 映射到 Strain。
	 *
	 * 实际项目中建议再做一层：
	 * Damage
	 *    ↓
	 * DestructionPower
	 *    ↓
	 * Strain
	 */
	Params.Strain = Damage;

	return ApplyImpact(Params);
}

UGeometryCollectionComponent* ULrDestructionComponent::GetGeometryCollection() const
{
	return GeometryCollection;
}

void ULrDestructionComponent::SetGeometryCollection(UGeometryCollectionComponent* InGeometryCollection)
{
	GeometryCollection = InGeometryCollection;
}

void ULrDestructionComponent::ResolveGeometryCollection()
{
	if (!GetOwner())
	{
		return;
	}

	GeometryCollection = GetOwner()->FindComponentByClass<UGeometryCollectionComponent>();
}

bool ULrDestructionComponent::IsValidGeometryCollection() const
{
	if (!IsValid(GeometryCollection))
	{
		return false;
	}

	return GeometryCollection->GetOwner() != nullptr;
}

FVector ULrDestructionComponent::CalculateBreakVelocity(const FLrDestructionImpactParams& Params, int32 ItemIndex) const
{
	FVector Forward = Params.ImpactDirection.GetSafeNormal();

	if (Forward.IsNearlyZero())
	{
		Forward = FVector::ForwardVector;
	}
	FVector Velocity = Forward * Params.ForwardSpeed;

	/**
	 * 径向散射。
	 * 注意：
	 *
	 * 这里没有碎片世界位置，
	 * 因此第一版采用稳定随机方向。
	 *
	 * 如果需要真正的：
	 *   HitPoint
	 *       ↓
	 *   FragmentPosition
	 *       ↓
	 *   RadialDirection
	 *
	 * 则应该改用 Physics Field，
	 * 后面我会给你第二版。
	 */
	if (Params.bUseRadialSpread && Params.RadialSpeed > 0.0f)
	{
		FRandomStream RandomStream(Params.RandomSeed + ItemIndex);
		const FVector RandomDir = RandomStream.VRand();

		/**
		 * 去掉 Forward 方向上的分量，
		 * 得到与子弹方向垂直的方向。
		 */
		FVector SideDir = RandomDir - Forward * FVector::DotProduct(RandomDir, Forward);
		SideDir.Normalize();
		if (!SideDir.IsNearlyZero())
		{
			Velocity += SideDir * Params.RadialSpeed;
		}
	}

	/**
	 * 随机扰动。
	 */
	if (Params.bUseRandomSpread && Params.RandomSpeed > 0.0f)
	{
		FRandomStream RandomStream(Params.RandomSeed + ItemIndex + 10000);
		Velocity += RandomStream.VRand() * Params.RandomSpeed;
	}

	return Velocity;
}

int32 ULrDestructionComponent::GetHitItemIndex(const FHitResult& Hit) const
{
	/**
	 * Geometry Collection 的 ItemIndex
	 * 可以从 HitResult::Item 获取。
	 *
	 * UE 的 ApplyExternalStrain API 也明确要求
	 * ItemIndex 来自 HitResult。
	 */
	return Hit.Item;
}

bool ULrDestructionComponent::ApplyStrain(int32 ItemIndex, const FLrDestructionImpactParams& Params)
{
	if (!IsValidGeometryCollection())
	{
		return false;
	}

	/**
	 * UE5.8：
	 *
	 * ApplyExternalStrain(
	 *     ItemIndex,
	 *     Location,
	 *     Radius,
	 *     PropagationDepth,
	 *     PropagationFactor,
	 *     Strain
	 * )
	 *
	 * 官方 API 用于对 Geometry Collection
	 * 的指定 Piece / Cluster 应用 External Strain。
	 */
	GeometryCollection->ApplyExternalStrain(
		ItemIndex,
		Params.HitPoint,
		Params.BreakRadius,
		Params.PropagationDepth,
		Params.PropagationFactor,
		Params.Strain);

	return true;
}

bool ULrDestructionComponent::ApplyBreakingVelocity(int32 ItemIndex, const FLrDestructionImpactParams& Params)
{
	if (!IsValidGeometryCollection())
	{
		return false;
	}
	const FVector Velocity = CalculateBreakVelocity(Params, ItemIndex);

	/**
	 * UE5.8:
	 *
	 * ApplyBreakingLinearVelocity()
	 *
	 * 对 Cluster 在发生 Break 时
	 * 释放出来的 pieces 设置线速度。
	 *
	 * 这是非常适合：
	 *     Bullet
	 *        ↓
	 *     Cluster Break
	 *        ↓
	 *     Pieces 飞出去
	 *
	 * 的 API。
	 */
	GeometryCollection->ApplyBreakingLinearVelocity(ItemIndex, Velocity);
	return true;
}
