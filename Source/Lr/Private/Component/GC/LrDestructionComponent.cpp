/**
 * LrDestructionComponent.cpp
 *
 * Geometry Collection 破坏组件实现。
 *
 * 设计思路（两阶段破坏）：
 *   1. ApplyExternalStrain：在 HitPoint 附近按 BreakRadius / Strain 触发 Cluster 破坏。
 *   2. ApplyBreakingLinearVelocity：给破坏瞬间释放的碎片设置初速度，使其沿 ImpactDirection 飞散。
 *
 * 调用链示例：
 *   子弹命中 -> ApplyBulletImpact(Hit, BulletVelocity, Damage)
 *     -> ApplyImpact(Params)
 *       -> ApplyStrain()             // 破坏
 *       -> ApplyBreakingVelocity()   // 抛射碎片
 */

#include "Component/GC/LrDestructionComponent.h"

#include "GeometryCollection/GeometryCollectionComponent.h"

// 构造函数：设置组件默认值
// - 关闭 Tick（破坏是事件驱动，不需要每帧更新）
// - 初始化 DefaultImpactParams，提供蓝图/代码开箱即用的破坏手感
ULrDestructionComponent::ULrDestructionComponent()
{
	// 破坏组件为事件驱动型，不需要 Tick，关闭以节省性能。
	PrimaryComponentTick.bCanEverTick = false;
	
	// 默认撞击点：原点；默认方向：前向。实际调用时会被真实命中数据覆盖。
	DefaultImpactParams.HitPoint = FVector::ZeroVector;
	DefaultImpactParams.ImpactDirection = FVector::ForwardVector;

	// 默认破坏强度与影响半径：Strain 越大越容易碎，BreakRadius 越大影响范围越广。
	DefaultImpactParams.Strain = 100.0f;
	DefaultImpactParams.BreakRadius = 100.0f;

	// Cluster 破坏传播参数：深度 1 表示只影响命中层级及下一层，0.5 为层间衰减系数。
	DefaultImpactParams.PropagationDepth = 1;
	DefaultImpactParams.PropagationFactor = 0.5f;

	// 碎片初速度：前向速度主导飞行方向（cm/s），径向/随机速度增加散射感。
	DefaultImpactParams.ForwardSpeed = 1200.0f;
	DefaultImpactParams.RadialSpeed = 250.0f;
	DefaultImpactParams.RandomSpeed = 100.0f;

	// 默认开启径向散射与随机扰动，避免碎片飞成一条直线。
	DefaultImpactParams.bUseRadialSpread = true;
	DefaultImpactParams.bUseRandomSpread = true;

	// 默认随机种子，保证同参数下碎片散射方向稳定可复现。
	DefaultImpactParams.RandomSeed = 1337;
	// ...
}


// BeginPlay：游戏开始时自动绑定 GeometryCollection
// 若 bAutoFindGeometryCollection 为 true 且外部未手动指定，则从 Owner 上查找同 Actor 的 GeometryCollection 组件。
void ULrDestructionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoFindGeometryCollection && !GeometryCollection)
	{
		ResolveGeometryCollection();
	}
}

/**
 * 通用破坏入口：子弹 / 近战 / 投射物 / 爆炸均可调用。
 * @param Params 破坏参数（撞击点、方向、强度、半径、速度等）
 * @return true 已成功发送破坏请求，false 参数非法或组件状态不允许破坏
 *
 * 流程：
 *   1. 参数合法性校验（GC 有效性、重复打击开关、半径/应变、方向归一化）
 *   2. ApplyStrain：触发 Cluster 破坏
 *   3. ApplyBreakingVelocity：设置碎片飞散初速度
 */
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

/**
 * 简化版破坏接口：基于默认参数，只覆盖关键四项。
 * @param HitPoint 撞击点（世界坐标）
 * @param ImpactDirection 撞击方向（内部会归一化），决定碎片主飞行方向
 * @param ForwardSpeed 碎片前向初速度，cm/s
 * @param BreakRadius 破坏影响半径
 * @return 同 ApplyImpact
 */
bool ULrDestructionComponent::ApplyImpactSimple(const FVector& HitPoint, const FVector& ImpactDirection, float ForwardSpeed, float BreakRadius)
{
	FLrDestructionImpactParams Params = DefaultImpactParams;

	Params.HitPoint = HitPoint;
	Params.ImpactDirection = ImpactDirection.GetSafeNormal();
	Params.ForwardSpeed = ForwardSpeed;
	Params.BreakRadius = BreakRadius;

	return ApplyImpact(Params);
}

/**
 * 子弹命中专用接口：从开火数据构造破坏参数。
 * @param Hit 子弹碰撞的 FHitResult（提供 ImpactPoint / ImpactNormal）
 * @param BulletVelocity 子弹速度矢量，用于推导冲击方向；为零时退化为 -ImpactNormal
 * @param Damage 伤害值，直接映射为 Strain（破坏强度）
 * @return 同 ApplyImpact
 */
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

// 获取当前绑定的 GeometryCollection 组件（可为空，调用前建议用 IsValidGeometryCollection 校验）。
UGeometryCollectionComponent* ULrDestructionComponent::GetGeometryCollection() const
{
	return GeometryCollection;
}

// 手动指定 GeometryCollection，用于 Owner 身上有多个 GC 或 GC 在其他 Actor 上的情况。
void ULrDestructionComponent::SetGeometryCollection(UGeometryCollectionComponent* InGeometryCollection)
{
	GeometryCollection = InGeometryCollection;
}

// 从 Owner 上自动查找第一个 UGeometryCollectionComponent 并缓存；Owner 为空时直接返回。
void ULrDestructionComponent::ResolveGeometryCollection()
{
	if (!GetOwner())
	{
		return;
	}

	GeometryCollection = GetOwner()->FindComponentByClass<UGeometryCollectionComponent>();
}

// 校验 GC 是否可用：指针有效且已有 Owner（已注册到世界中）。
bool ULrDestructionComponent::IsValidGeometryCollection() const
{
	if (!IsValid(GeometryCollection))
	{
		return false;
	}

	return GeometryCollection->GetOwner() != nullptr;
}

/**
 * 计算碎片破坏瞬间初速度。
 * @param Params 破坏参数（含方向、前向/径向/随机速度及开关）
 * @param ItemIndex 触发破坏的 Cluster 索引，用于构造稳定随机流（INDEX_NONE 时为通用场影响）
 * @return 世界空间线速度（cm/s），合成公式：
 *   Velocity = Forward * ForwardSpeed + SideDir * RadialSpeed + RandomDir * RandomSpeed
 */
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

// 从 HitResult 提取 GeometryCollection 的 ItemIndex（即 Hit.Item），供 ApplyExternalStrain 精确定位 Piece。
// 注意：当前 ApplyImpact 走通用场影响路径（INDEX_NONE），此函数为后续精确命中扩展预留。
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

/**
 * 第一阶段：施加外部应变，触发 Cluster 破坏。
 * @param ItemIndex 目标索引，INDEX_NONE 表示按 Location + Radius 做场影响
 * @param Params 破坏参数（HitPoint / BreakRadius / PropagationDepth / PropagationFactor / Strain）
 * @return GC 有效时恒为 true（UE 的 ApplyExternalStrain 本身无返回值）
 */
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

/**
 * 第二阶段：设置 Cluster 破坏时释放碎片的线速度。
 * @param ItemIndex 目标索引，与 ApplyStrain 保持一致
 * @param Params 破坏参数，内部经 CalculateBreakVelocity 合成最终速度
 * @return GC 有效时恒为 true
 */
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
