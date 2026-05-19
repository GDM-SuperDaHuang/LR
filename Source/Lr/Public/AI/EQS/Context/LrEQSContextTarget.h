// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "LrEQSContextTarget.generated.h"

/**
 * EQS 上下文：以黑板的 TargetActor 作为查询中心
 *
 * 当 EQS 查询需要围绕当前目标生成测试点时，此上下文从 AI 控制器的黑板中
 * 读取 TargetActor 键（由 LrAIController::OnTargetDetected 写入），
 * 将其位置作为查询的环境上下文返回
 */
UCLASS()
class LR_API ULrEQSContextTarget : public UEnvQueryContext
{
	GENERATED_BODY()

protected:
	/**
	 * 提供查询上下文数据
	 * 从行为树黑板中读取 TargetActor，将其作为单个 Actor 项返回
	 */
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};