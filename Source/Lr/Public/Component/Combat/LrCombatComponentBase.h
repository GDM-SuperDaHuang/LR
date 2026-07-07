// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LrCombatComponentBase.generated.h"


UENUM(BlueprintType)
enum class ELrCombatShape : uint8
{
	ConeOne,
	Sphere,
	Box,
	Cone
};

USTRUCT(BlueprintType)
struct FLrCombatQueryParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELrCombatShape ShapeType = ELrCombatShape::Sphere;

	/** * [核心参数] 检测的起点/原点（世界坐标）
	 * - 如果是以自身为中心释放：传入释放者的 GetActorLocation()
	 * - 如果是远程技能落点：传入目标的受击点或投射物（Projectile）的位置
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;

	/** * [核心参数] 检测的朝向/正前方（归一化的方向向量）
	 * - 仅在 Box 和 Cone 形状下生效，用于确定盒子向前延伸的方向或扇形的中心轴
	 * - 常见赋值：释放者的 GetActorForwardVector()
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Forward = FVector::ForwardVector;

	// ==================== Sphere (球体配置) ====================
	/** * 球体的检测半径
	 * - 仅当 ShapeType == Sphere 时生效
	 * - 数值越大，周身受击范围越大
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 300.f;

	// ==================== Box (长方体配置) ====================
	/** * 长方体的半范围（Half-Extent，即从中心点到各边界的距离）
	 * - 仅当 ShapeType == Box 时生效
	 * - 解释：FVector(半长, 半宽, 半高)。
	 * - 注意：由于你的代码中 BoxCenter = Origin + Forward * BoxExtent.X，
	 * 意味着盒子的实际前方攻击距离是 BoxExtent.X * 2，左右总宽度是 BoxExtent.Y * 2
	 * 如果你希望做一个长 400、宽 200、高 200 的矩形检测区域：
	 * 你的 BoxExtent 应该填写为：FVector(200.f, 100.f, 100.f)（全部除以 2）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BoxExtent = FVector(100.f);


	// ==================== Cone (扇形/锥形配置) ====================
	/** * 扇形的半角度（度数 Degree）
	 * - 仅当 ShapeType == Cone 时生效
	 * - ⚠️ 重点：这是【中心对称轴到一侧边界】的角度。
	 * - 示例：如果你想做一个 90 度的扇形顺劈斩，这里应该填 45.f
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConeHalfAngle = 80.f;
	/** * 扇形的攻击距离/长度
	 * - 仅当 ShapeType == Cone 时生效
	 * - 决定了扇形能打多远（底层实际上是先做一个该长度为半径的球体筛选，再判断角度）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConeLength = 500.f;

	// ==================== Filter & Debug (过滤与调试) ====================
	/** 是否忽略释放者自身（防止 AOE 技能或者近战挥砍伤害到自己） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIgnoreSelf = true;

	/** 是否忽略同队队友（用于实现“友军免伤”逻辑，内部依赖 TeamID 比较） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIgnoreTeammates = true;

	/** 是否在场景中绘制 Debug 调试线（球体/盒子/锥形）
	 * - 开启后会在编辑器中实时画出绿/蓝/红色的检测范围，非常便于美术和策划对齐特效
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrawDebug = false;

	/**
	 *Origin 与 Forward 的联动
	 *这两个参数决定了攻击的绝对空间位置。例如：
	 *战士挥剑：Origin = 战士.GetActorLocation(), Forward = 战士.GetActorForwardVector()。
	 *法师在远处脚下放烈焰风暴：Origin = 魔法阵中心世界坐标, Forward = FVector::ForwardVector（此时如果是 Sphere，Forward 无所谓；如果是矩形法阵，Forward 决定法阵的旋转朝向）。
	 */
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrCombatComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	ULrCombatComponentBase();
	
	// 新增：获取前方扇形范围内最近的一个敌人（会自动更新 CachedTargetActor）
	virtual TWeakObjectPtr<AActor> GetClosestEnemyInCone(const FLrCombatQueryParams& Params);
	// AOE 类型
	TArray<TWeakObjectPtr<AActor>> PerformCombatQuery(const FLrCombatQueryParams& Params);
	TArray<TWeakObjectPtr<AActor>> PerformSphereQuery(const FLrCombatQueryParams& Params); //圆形
	TArray<TWeakObjectPtr<AActor>> PerformBoxQuery(const FLrCombatQueryParams& Params); //矩形
	TArray<TWeakObjectPtr<AActor>> PerformConeQuery(const FLrCombatQueryParams& Params); //扇形
	// 目标过滤
	bool IsValidCombatTarget(AActor* TargetActor, const FLrCombatQueryParams& Params) const;
	// 角度判断
	bool IsInsideCone(const FVector& Origin, const FVector& Forward, const FVector& TargetLocation, float ConeHalfAngleDeg) const;

	// 距离排序
	static void SortActorsByDistance(TArray<TWeakObjectPtr<AActor>>& InOutActors, const FVector& Origin);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
