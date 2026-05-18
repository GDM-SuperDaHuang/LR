// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LrBTTaskRunEQS.generated.h"

struct FEnvQueryResult;
class UEnvQuery;

/**
 * 运行 EQS 查询任务
 * 异步执行指定的环境查询，将最优结果位置写入黑板
 */
UCLASS()
class LR_API ULrBTTaskRunEQS : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULrBTTaskRunEQS();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** EQS 查询完成回调 */
	void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result);

public:
	/** 要执行的 EQS 查询模板 */
	UPROPERTY(EditAnywhere, Category = "EQS")
	TObjectPtr<UEnvQuery> QueryTemplate;

	/** 查询结果写入的黑板键名 */
	UPROPERTY(EditAnywhere, Category = "EQS")
	FName BlackboardKey;

private:
	/** 缓存的 OwnerComp 指针，用于异步回调中完成任务 */
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};