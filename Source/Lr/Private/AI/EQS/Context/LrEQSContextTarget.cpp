// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/Context/LrEQSContextTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Lr/Lr.h"

void ULrEQSContextTarget::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

	// 从 EQS 查询实例的 Owner 获取 AI 控制器
	// QueryInstance.Owner 在行为树驱动的 EQS 查询中即为 AI 控制器
	AAIController* AIController = Cast<AAIController>(QueryInstance.Owner.Get());
	if (!AIController)
	{
		return;
	}

	// 获取 AI 控制器关联的黑板组件
	UBlackboardComponent* BB = AIController->GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	// 从黑板读取当前感知到的目标 Actor（由 OnTargetDetected 写入）
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(LrBBKeys::TargetActor));
	if (!Target)
	{
		return;
	}

	// 将目标 Actor 设置为 EQS 查询的环境上下文
	// EQS Generator 可以使用此上下文围绕目标生成测试点
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, Target);
}