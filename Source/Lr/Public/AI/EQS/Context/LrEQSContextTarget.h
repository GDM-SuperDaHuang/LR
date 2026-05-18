// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "LrEQSContextTarget.generated.h"

/**
 * EQS 上下文：以黑板的 TargetActor 作为查询中心
 * 用于 EQS 查询时围绕当前检测到的目标生成测试点
 */
UCLASS()
class LR_API ULrEQSContextTarget : public UEnvQueryContext
{
	GENERATED_BODY()

protected:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};