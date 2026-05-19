// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/Generator/LrEQSGeneratorCircle.h"

#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"

void ULrEQSGeneratorCircle::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	// 获取查询者（Querier）的当前位置作为圆心
	// QueryInstance.Owner 即为执行此 EQS 的 AI 控制器
	TArray<FVector> ContextLocations;
	QueryInstance.PrepareContext(UEnvQueryContext_Querier::StaticClass(), ContextLocations);

	if (ContextLocations.IsEmpty())
	{
		return;
	}

	const FVector Center = ContextLocations[0];
	TArray<FNavLocation> ItemCandidates;

	// 计算每个点之间的角度步长（360度 / 点数）
	const float StepAngle = 360.f / PointCount;

	// 在圆上均匀生成测试点
	for (int32 i = 0; i < PointCount; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * StepAngle);

		FVector Offset;
		Offset.X = FMath::Cos(Angle) * Radius;
		Offset.Y = FMath::Sin(Angle) * Radius;
		Offset.Z = 0.f;

		ItemCandidates.Add(FNavLocation(Center + Offset));
	}

	// 投影到 NavMesh 上，过滤掉不在导航网格上的点
	// 这确保返回的测试点 AI 可以实际到达
	ProjectAndFilterNavPoints(ItemCandidates, QueryInstance);

	// 将最终有效的测试点存入查询实例
	StoreNavPoints(ItemCandidates, QueryInstance);
}