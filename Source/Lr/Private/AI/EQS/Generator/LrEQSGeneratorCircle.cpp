// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/Generator/LrEQSGeneratorCircle.h"

#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"

// 静态成员初始化
ULrEQSGeneratorCircle::FCirclePointCache ULrEQSGeneratorCircle::PointCache;
uint32 ULrEQSGeneratorCircle::CurrentFrameCounter = 0;

void ULrEQSGeneratorCircle::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	// 获取查询者（Querier）的当前位置作为圆心
	TArray<FVector> ContextLocations;
	QueryInstance.PrepareContext(UEnvQueryContext_Querier::StaticClass(), ContextLocations);

	if (ContextLocations.IsEmpty())
	{
		return;
	}

	const FVector Center = ContextLocations[0];
	TArray<FNavLocation> ItemCandidates;

	// 尝试使用缓存（当大量 AI 围绕同一目标查询时，避免重复计算）
	bool bUsedCache = false;
	if (bEnablePointCache)
	{
		CurrentFrameCounter = GFrameCounter;
		const float RadiusTolerance = 1.0f;
		const float CenterTolerance = 10.0f;

		if (PointCache.FrameCounter >= CurrentFrameCounter - static_cast<uint32>(CacheValidFrames) &&
		    PointCache.FrameCounter <= CurrentFrameCounter &&
		    FMath::Abs(PointCache.CachedRadius - Radius) < RadiusTolerance &&
		    (PointCache.Center - Center).SizeSquared() < FMath::Square(CenterTolerance))
		{
			// 缓存命中，直接复用
			ItemCandidates = PointCache.Points;
			bUsedCache = true;
		}
	}

	if (!bUsedCache)
	{
		// 预分配数组容量，避免动态扩容
		ItemCandidates.Reserve(PointCount);
		GenerateCirclePoints(ItemCandidates, Center);

		// 写入缓存
		if (bEnablePointCache)
		{
			PointCache.Center = Center;
			PointCache.CachedRadius = Radius;
			PointCache.Points = ItemCandidates;
			PointCache.FrameCounter = CurrentFrameCounter;
		}
	}

	// 投影到 NavMesh 上，过滤掉不在导航网格上的点
	ProjectAndFilterNavPoints(ItemCandidates, QueryInstance);

	// 将最终有效的测试点存入查询实例
	StoreNavPoints(ItemCandidates, QueryInstance);
}

void ULrEQSGeneratorCircle::GenerateCirclePoints(TArray<FNavLocation>& OutPoints, const FVector& Center) const
{
	// 计算每个点之间的角度步长（360度 / 点数）
	const float StepAngle = 360.f / PointCount;
	const float DegToRad = PI / 180.f;

	// 批量计算，避免重复三角函数调用
	for (int32 i = 0; i < PointCount; ++i)
	{
		const float AngleRad = i * StepAngle * DegToRad;
		const float CosVal = FMath::Cos(AngleRad);
		const float SinVal = FMath::Sin(AngleRad);

		FVector Offset;
		Offset.X = CosVal * Radius;
		Offset.Y = SinVal * Radius;
		Offset.Z = 0.f;

		OutPoints.Emplace(Center + Offset);
	}
}
