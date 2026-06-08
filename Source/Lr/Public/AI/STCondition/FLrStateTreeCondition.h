#pragma once
#include "EngineUtils.h"
#include "StateTreeConditionBase.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "AI/Evaluator/FLrStateTreeEvaluator.h"
#include "FLrStateTreeCondition.generated.h"


USTRUCT(BlueprintType)
struct FLrStateTreeCondition : public FStateTreeConditionBase
{
	GENERATED_BODY()
	
	// using FInstanceDataType = FLrEvaluatorInstanceData;
	
	// 声明 Handle 读取外部数据
	// TStateTreeExternalDataHandle<FEnemyAIExternalData> EnemyDataHandle;

	// virtual bool Link(FStateTreeLinker& Linker) override
	// {
	// 	Linker.LinkExternalData(EnemyDataHandle);
	// 	return true;
	// }

	// virtual bool TestCondition(FStateTreeExecutionContext& Context) const override
	// {
	// 	FEnemyAIExternalData& Data = Context.GetExternalData(EnemyDataHandle);
	// 	if (!Data.SelfPawn || !Data.TargetActor)
	// 		return false;
	//
	// 	float Dist = FVector::Dist(Data.SelfPawn->GetActorLocation(), Data.TargetActor->GetActorLocation());
	// 	return Dist <= Data.AttackRange;
	// }
};


USTRUCT()
struct FLrAttackConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere)
	float AttackRange = 300.f;
};


USTRUCT(meta=(DisplayName="In Attack Range"))
struct FLrAttackCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FLrAttackConditionInstanceData;
	
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override
	{
		const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		if (!InstanceData.TargetActor)
		{
			return false;
		}

		return true;
	}
};
