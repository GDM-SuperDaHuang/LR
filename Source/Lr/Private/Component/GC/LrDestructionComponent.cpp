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
 *     -> ApplyImpactWithResult(BreakAndFall, ...)
 *       -> ApplyImpactWithItem()
 *         -> ApplyStrain()             // 破坏
 *         -> ApplyBreakingVelocity()   // 抛射碎片
 *
 *   其他来源 -> ApplyImpactWithResult(Nothing / ToppleOnly / BreakAndFall, ...)
 *     ToppleOnly -> StartScriptedTopple()  // 脚本化倾倒（睡眠 GC 不响应速度命令，但跟随 Actor 变换）
 */

#include "Component/GC/LrDestructionComponent.h"

#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

// 构造函数：设置组件默认值
// - 平时不 Tick（破坏是事件驱动）；仅脚本化倾倒（ToppleOnly）期间临时开启
// - 初始化 DefaultImpactParams，提供蓝图/代码开箱即用的破坏手感
ULrDestructionComponent::ULrDestructionComponent()
{
	// 破坏组件平时不 Tick，只在脚本化倾倒期间启用（见 StartScriptedTopple）。
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
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

	ApplyDamageSettings();
}

// 覆盖 GC 的破坏模型与阈值：
// GC 资产默认 DamageThreshold = { 500000, 50000, 5000 }，
// 而本组件用 Damage/Strain 的 100 级手感数值，不覆盖则 Strain 永远达不到阈值。
void ULrDestructionComponent::ApplyDamageSettings()
{
	if (!bOverrideDamageThreshold || !IsValidGeometryCollection())
	{
		return;
	}

	GeometryCollection->SetDamageModel(EDamageModelTypeEnum::Chaos_Damage_Model_UserDefined_Damage_Threshold);
	GeometryCollection->SetDamageThreshold(DamageThreshold);

	UE_LOG(LogTemp, Log, TEXT("LrDestruction: DamageThreshold overridden to %s."), *FString::JoinBy(DamageThreshold, TEXT(", "), [](float Value) { return FString::SanitizeFloat(Value); }));
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
	/**
	 * 通用入口没有 ItemIndex，传 INDEX_NONE；
	 * ApplyImpactWithItem 内部会退化为根 Cluster 索引，
	 * 由引擎按 Location + Radius 影响附近子块。
	 */
	return ApplyImpactWithItem(INDEX_NONE, Params);
}

bool ULrDestructionComponent::ApplyImpactWithItem(int32 ItemIndex, const FLrDestructionImpactParams& Params)
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

	// 睡眠中的 GC 需要保持 Simulate/Gravity，碎裂判定本身会唤醒命中范围内的子块。
	WakeUpForImpact();

	/**
	 * ApplyExternalStrain / ApplyBreakingLinearVelocity 的 ItemIndex
	 * 必须是有效碎片索引（来自 HitResult.Item）：
	 * 传 INDEX_NONE 时引擎找不到对应 Handle，整条命令会被静默丢弃。
	 * 无命中信息时退化为根 Cluster，按 Location + Radius 影响附近子块。
	 */
	if (ItemIndex < 0)
	{
		ItemIndex = GeometryCollection->GetRootIndex();
	}
	if (ItemIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("LrDestruction: GeometryCollection has no valid RootIndex."));
		return false;
	}

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
 * 子弹命中专用接口：等价于 ApplyImpactWithResult(BreakAndFall, ...)。
 * @param Hit 子弹碰撞的 FHitResult（提供 ImpactPoint / ImpactNormal）
 * @param BulletVelocity 子弹速度矢量，用于推导冲击方向；为零时退化为 -ImpactNormal
 * @param Damage 伤害值，直接映射为 Strain（破坏强度）
 * @return 同 ApplyImpact
 */
bool ULrDestructionComponent::ApplyBulletImpact(const FHitResult& Hit, const FVector& BulletVelocity, float Damage)
{
	return ApplyImpactWithResult(ELrDestructionResult::BreakAndFall, Hit, BulletVelocity, Damage);
}

/**
 * 按结果选择破坏方式的统一入口。
 *
 * Result == Nothing      ：不唤醒、不产生 Strain、不施加速度，直接返回。
 * Result == ToppleOnly   ：脚本化倾倒（沿冲击方向绕底缘旋转 90°），不产生 Strain。
 * Result == BreakAndFall ：构造 Strain=Damage 的参数走 ApplyImpactWithItem（子弹路径）。
 */
bool ULrDestructionComponent::ApplyImpactWithResult(ELrDestructionResult Result, const FHitResult& Hit, const FVector& ImpactVelocity, float Damage, float ToppleDuration)
{
	if (!IsValidGeometryCollection())
	{
		UE_LOG(LogTemp, Warning, TEXT("LrDestruction: GeometryCollection is invalid."));
		return false;
	}

	// 3: 不倒下也不碎裂：什么都不做。
	// 若碰撞通道是 Block，物理接触本身仍可能推动 GC；需要完全无反应时走通道 Ignore / 游戏层拦截。
	if (Result == ELrDestructionResult::Nothing)
	{
		return true;
	}

	const FVector Direction = ResolveImpactDirection(Hit, ImpactVelocity);

	// 1: 倒下但不碎裂：脚本化倾倒（睡眠 GC 粒子不响应速度/冲量命令，但跟随 Actor 变换）。
	if (Result == ELrDestructionResult::ToppleOnly)
	{
		StartScriptedTopple(Direction, ToppleDuration);

		UE_LOG(LogTemp, Log, TEXT("LrDestruction: ToppleOnly loc=%s dir=%s duration=%.2f."), *Hit.ImpactPoint.ToString(), *Direction.ToString(), ToppleDuration);
		return true;
	}

	// 2: 倒下并碎裂：原子弹路径。
	// Overlap 回调的 SweepResult 可能没有 ImpactPoint / ImpactNormal，做兜底。
	FVector HitPoint = Hit.ImpactPoint;
	if (HitPoint.IsNearlyZero())
	{
		HitPoint = Hit.Location;
	}
	if (HitPoint.IsNearlyZero())
	{
		HitPoint = GeometryCollection->GetComponentLocation();
	}

	FLrDestructionImpactParams Params = DefaultImpactParams;

	Params.HitPoint = HitPoint;

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

	// 有精确 Item（Block Hit）就打精确 Piece，否则按 Location + Radius 场影响。
	const int32 ItemIndex = Hit.Item >= 0 ? Hit.Item : INDEX_NONE;
	UE_LOG(LogTemp, Warning, TEXT("BulletHit Item=%d bBlocking=%d ImpactPoint=%s Damage=%f"),Hit.Item, Hit.bBlockingHit, *Hit.ImpactPoint.ToString(), Damage);
	return ApplyImpactWithItem(ItemIndex, Params);
}

// 解析冲击方向：ImpactVelocity 优先，其次 -ImpactNormal，再退回 DefaultImpactParams.ImpactDirection，最后 Fallback 前向。
FVector ULrDestructionComponent::ResolveImpactDirection(const FHitResult& Hit, const FVector& ImpactVelocity) const
{
	FVector Direction = ImpactVelocity.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = (-Hit.ImpactNormal).GetSafeNormal();
	}
	if (Direction.IsNearlyZero())
	{
		Direction = DefaultImpactParams.ImpactDirection.GetSafeNormal();
	}
	if (Direction.IsNearlyZero())
	{
		Direction = FVector::ForwardVector;
	}
	return Direction;
}

bool ULrDestructionComponent::WakeUpForImpact()
{
	if (!IsValidGeometryCollection())
	{
		return false;
	}

	// 必须 Simulate 才能 Break / 飞碎片。
	GeometryCollection->SetSimulatePhysics(true);
	// 保持 DynamicCollection 状态为 Dynamic（仅在物理状态重建时生效；碎裂的实际唤醒靠 ApplyBreakingLinearVelocity）。
	GeometryCollection->SetDynamicState(Chaos::EObjectStateType::Dynamic);
	GeometryCollection->SetEnableGravity(true);
	GeometryCollection->WakeRigidBody();

	return true;
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

// Tick 仅在脚本化倾倒期间生效（平时 bStartWithTickEnabled=false）。
void ULrDestructionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bToppling)
	{
		TickScriptedTopple(DeltaTime);
	}
}

/**
 * 启动脚本化倾倒（ToppleOnly）。
 *
 * 背景：睡眠 GC（Dynamic + StartAwake=false）的粒子不响应
 * ApplyLinearVelocity / ApplyAngularVelocity / ApplyImpulseAtLocation / WakeParticle
 * 等唤醒命令（UE5.8 引擎行为，已实测：命令送达但粒子保持睡眠冻结），
 * 但粒子会跟随 Actor（Component）变换（已实测）。因此用脚本化旋转实现“倒而不碎”：
 *
 *   - 旋转增量：FindBetweenNormals(Up, Direction)，即把局部 +Z 倒向冲击方向；
 *   - 轴心：几何体底部、倾倒方向一侧的边缘（世界坐标），保证旋转过程不插入地面；
 *   - 曲线：t^2 重力式加速，末端直接停在 90°（倒平）。
 */
void ULrDestructionComponent::StartScriptedTopple(const FVector& Direction, float Duration)
{
	AActor* Owner = GetOwner();
	if (!Owner || !IsValidGeometryCollection())
	{
		return;
	}

	// 只取水平方向参与倾倒；水平分量为零时退化为 Actor 前向。
	FVector DirH(Direction.X, Direction.Y, 0.0f);
	if (!DirH.Normalize())
	{
		DirH = Owner->GetActorForwardVector().GetSafeNormal2D();
	}
	if (DirH.IsNearlyZero())
	{
		return;
	}

	// 轴心 = 几何体底部、倾倒方向一侧的边缘（世界坐标，取 GC 当前包围盒）。
	const FBoxSphereBounds Bounds = GeometryCollection->Bounds;
	const FVector Center = Bounds.Origin;
	const FVector Extent = Bounds.BoxExtent;

	TopplePivot.X = DirH.X > 0.0f ? Center.X + Extent.X : Center.X - Extent.X;
	TopplePivot.Y = DirH.Y > 0.0f ? Center.Y + Extent.Y : Center.Y - Extent.Y;
	TopplePivot.Z = Center.Z - Extent.Z;

	ToppleStartQuat = Owner->GetActorQuat();
	ToppleStartOffset = Owner->GetActorLocation() - TopplePivot;

	// FindBetweenNormals 避免手写轴角的方向符号歧义：局部 Up 恰好倒向 DirH。
	ToppleDeltaQuat = FQuat::FindBetweenNormals(FVector::UpVector, DirH);

	ToppleElapsedSeconds = 0.0f;
	ToppleDurationSeconds = FMath::Max(Duration, 0.05f);
	bToppling = true;

	SetComponentTickEnabled(true);
}

/**
 * 推进脚本化倾倒：Delta(t) = Slerp(I, ToppleDeltaQuat, t^2)。
 *   Rotation(t) = Delta(t) * StartQuat
 *   Location(t) = Pivot + Delta(t) * (StartLoc - Pivot)
 * 倾倒完成后停止 Tick，Actor 停在倒平的姿态（GC 仍为睡眠冻结，不产生碎裂）。
 */
void ULrDestructionComponent::TickScriptedTopple(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		bToppling = false;
		SetComponentTickEnabled(false);
		return;
	}

	ToppleElapsedSeconds += DeltaTime;
	const float Alpha = FMath::Clamp(ToppleElapsedSeconds / ToppleDurationSeconds, 0.0f, 1.0f);
	const float Eased = Alpha * Alpha; // 重力式加速

	const FQuat Delta = FQuat::Slerp(FQuat::Identity, ToppleDeltaQuat, Eased);

	Owner->SetActorRotation(Delta * ToppleStartQuat);
	Owner->SetActorLocation(TopplePivot + Delta.RotateVector(ToppleStartOffset), false, nullptr, ETeleportType::None);

	if (Alpha >= 1.0f)
	{
		bToppling = false;
		SetComponentTickEnabled(false);
	}
}
