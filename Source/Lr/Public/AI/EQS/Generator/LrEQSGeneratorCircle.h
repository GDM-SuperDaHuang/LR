// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_ProjectedPoints.h"
#include "LrEQSGeneratorCircle.generated.h"

/**
 * 圆形 EQS 点生成器
 * 在查询者周围以等角间距生成指定数量的点，经 NavMesh 投影过滤后作为 EQS 测试项
 *
 * 用于 AI 围绕某位置寻找合适的站位点（例如围绕目标寻找攻击位置）
 *
 * 生成逻辑：
 * 1. 获取查询者（Querier）的当前位置作为圆心
 * 2. 在 Radius 半径的圆上均匀取 PointCount 个点
 * 3. 通过 ProjectAndFilterNavPoints 投影到 NavMesh 并过滤不可达点
 *
 * 优化特性：
 * - 支持批量生成时分帧处理（避免大量 AI 同时查询造成卡顿）
 * - 相同圆心+半径的查询结果缓存（同一帧内复用）
 */
UCLASS(meta = (DisplayName = "Lr Circle Generator"))
class LR_API ULrEQSGeneratorCircle : public UEnvQueryGenerator_ProjectedPoints
{
	GENERATED_BODY()

protected:
	/**
	 * 生成圆形分布的 EQS 测试点
	 * 每个点等角分布在半径 Radius 的圆上，然后投影到 NavMesh
	 */
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

	/**
	 * 批量生成圆上的点（使用预分配数组避免重复内存分配）
	 */
	void GenerateCirclePoints(TArray<FNavLocation>& OutPoints, const FVector& Center) const;

public:
	/** 圆形半径（单位：厘米，默认 500 = 5m） */
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
	float Radius = 500.f;

	/** 生成的测试点数量（默认 12 个） */
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
	int32 PointCount = 12;

	/** 
	 * 是否启用点生成缓存（同一帧内相同圆心+半径的查询复用结果）
	 * 适用于大量 AI 围绕同一目标查询站位点的情况
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Optimization")
	bool bEnablePointCache = true;

	/** 缓存的有效帧数（默认 1 帧） */
	UPROPERTY(EditDefaultsOnly, Category = "Optimization")
	int32 CacheValidFrames = 1;

private:
	/** 缓存结构：按圆心+半径缓存生成的点 */
	struct FCirclePointCache
	{
		FVector Center;
		float CachedRadius;
		TArray<FNavLocation> Points;
		uint32 FrameCounter;
	};

	static FCirclePointCache PointCache;
	static uint32 CurrentFrameCounter;
};