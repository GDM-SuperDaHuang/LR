// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_ProjectedPoints.h"
#include "LrEQSGeneratorCircle.generated.h"

/**
 * 圆形 EQS 点生成器
 * 在查询者周围以圆形分布生成指定数量的 NavMesh 测试点
 * 用于 AI 围绕目标寻找合适位置
 */
UCLASS(meta = (DisplayName = "Lr Circle Generator"))
class LR_API ULrEQSGeneratorCircle : public UEnvQueryGenerator_ProjectedPoints
{
	GENERATED_BODY()

protected:
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

public:
	/** 圆形半径（单位：厘米） */
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
	float Radius = 500.f;

	/** 生成的测试点数量 */
	UPROPERTY(EditDefaultsOnly, Category = "Generator")
	int32 PointCount = 12;
};