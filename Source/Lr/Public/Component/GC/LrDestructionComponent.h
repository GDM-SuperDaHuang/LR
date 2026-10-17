// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LrDestructionComponent.generated.h"
class UGeometryCollectionComponent;

/**
 * Geometry Collection 破坏参数。
 *
 * 核心逻辑：
 *
 *   HitPoint
 *       ↓
 *   External Strain
 *       ↓
 *   Cluster Break
 *       ↓
 *   Breaking Linear Velocity
 *       ↓
 *   碎片沿 ImpactDirection 飞出
 */
USTRUCT(BlueprintType)
struct FLrDestructionImpactParams
{
	GENERATED_BODY()

	/** 撞击位置，世界坐标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	FVector HitPoint = FVector::ZeroVector;

	/**
	 * 撞击方向。
	 *
	 * 例如子弹：
	 *
	 * BulletVelocity.GetSafeNormal()
	 *
	 * 注意：
	 * 这个方向表示“物体受到冲击后碎片飞出去的主要方向”。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	FVector ImpactDirection = FVector::ForwardVector;

	/** 破坏强度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float Strain = 100.0f;

	/**
	 * Strain 影响半径。
	 *
	 * 注意 UE 的 ApplyExternalStrain 文档说明，
	 * Radius 是根据碎片质心到 Location 的距离进行影响。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float BreakRadius = 100.0f;

	/** Cluster 层级传播深度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	int32 PropagationDepth = 1;

	/** Cluster 层级传播衰减 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float PropagationFactor = 0.5f;

	/**
	 * 沿撞击方向的速度。
	 *
	 * 单位：cm/s
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float ForwardSpeed = 1200.0f;

	/**
	 * 径向散射速度。
	 *
	 * 用于让碎片不要全部飞成一条直线。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float RadialSpeed = 250.0f;

	/**
	 * 随机速度。
	 *
	 * 防止碎片运动完全一致。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	float RandomSpeed = 100.0f;

	/**
	 * 是否加入径向散射。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	bool bUseRadialSpread = true;

	/**
	 * 是否加入随机扰动。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	bool bUseRandomSpread = true;

	/**
	 * 随机数种子。
	 *
	 * 如果需要网络表现一致，可以由服务器生成固定 Seed。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	int32 RandomSeed = 1337;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrDestructionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULrDestructionComponent();

	/**
	 * Geometry Collection。
	 *
	 * 可以在 Actor 上自动查找，
	 * 也可以在蓝图中手动指定。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	TObjectPtr<UGeometryCollectionComponent> GeometryCollection;

	/**
	 * 是否允许重复破坏。
	 *
	 * false：
	 * 第一次破坏之后，后续 ApplyImpact 不再触发 Cluster Break。
	 *
	 * true：
	 * 可以继续对已经破碎的 Geometry Collection 施加影响。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	bool bAllowRepeatedImpact = true;

	/**
	* 是否自动查找 Geometry Collection。
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	bool bAutoFindGeometryCollection = true;

	/**
	 * 默认破坏参数。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	FLrDestructionImpactParams DefaultImpactParams;

	virtual void BeginPlay() override;

	/**
	 * 子弹 / 近战 / 投射物 / 爆炸统一调用接口。
	 *
	 * 返回：
	 * true  = 成功向 Geometry Collection 发送破坏
	 * false = 当前无法执行
	 */
	UFUNCTION(BlueprintCallable, Category = "LR|Destruction")
	bool ApplyImpact(const FLrDestructionImpactParams& Params);

	/**
	 * 简化接口。
	 *
	 * 例如：
	 *
	 * ApplyImpact(
	 *     Hit.ImpactPoint,
	 *     BulletVelocity.GetSafeNormal(),
	 *     1200.f,
	 *     100.f);
	 */
	UFUNCTION(BlueprintCallable, Category = "LR|Destruction")
	bool ApplyImpactSimple(const FVector& HitPoint, const FVector& ImpactDirection, float ForwardSpeed = 1200.0f, float BreakRadius = 100.0f);

	/**
	 * 子弹命中专用接口。
	 */
	UFUNCTION(BlueprintCallable, Category = "LR|Destruction")
	bool ApplyBulletImpact(const FHitResult& Hit, const FVector& BulletVelocity, float Damage = 100.0f);

	/**
	 * 唤醒 Geometry Collection 用于破坏。
	 *
	 * Static / Sleeping 的 GC 直接打 Strain 不会碎，
	 * 需要先切 Dynamic + 重力 + Simulate，才能 Cluster Break。
	 */
	UFUNCTION(BlueprintCallable, Category = "LR|Destruction")
	bool WakeUpForImpact();

	/**
	 * 获取 Geometry Collection。
	 */
	UFUNCTION(BlueprintPure, Category = "LR|Destruction")
	UGeometryCollectionComponent* GetGeometryCollection() const;

	/**
	 * 设置 Geometry Collection。
	 */
	UFUNCTION(BlueprintCallable, Category = "LR|Destruction")
	void SetGeometryCollection(UGeometryCollectionComponent* InGeometryCollection);

protected:
	/**
	 * 找到 Geometry Collection。
	 */
	void ResolveGeometryCollection();

	/**
	 * 检查当前 Geometry Collection 是否可以工作。
	 */
	bool IsValidGeometryCollection() const;

	/**
	 * 计算最终碎片初速度。
	 *
	 * Direction =
	 *
	 *   ImpactDirection * ForwardSpeed
	 *       +
	 *   RadialDirection * RadialSpeed
	 *       +
	 *   RandomDirection * RandomSpeed
	 */
	FVector CalculateBreakVelocity(const FLrDestructionImpactParams& Params, int32 ItemIndex) const;

	/**
	 * 获取 Hit 对应的 ItemIndex。
	 */
	int32 GetHitItemIndex(const FHitResult& Hit) const;

	/**
	 * 带 ItemIndex 的破坏入口（子弹精确命中用）。
	 * INDEX_NONE 时退化为按 Location + Radius 场影响。
	 */
	bool ApplyImpactWithItem(int32 ItemIndex, const FLrDestructionImpactParams& Params);

	/**
	 * 应用 Cluster 破坏。
	 */
	bool ApplyStrain(int32 ItemIndex, const FLrDestructionImpactParams& Params);

	/**
	 * 应用破碎时的初速度。
	 */
	bool ApplyBreakingVelocity(int32 ItemIndex, const FLrDestructionImpactParams& Params);

private:
	/**
	 * 是否已经执行过第一次破坏。
	 */
	bool bHasImpacted = false;
};
