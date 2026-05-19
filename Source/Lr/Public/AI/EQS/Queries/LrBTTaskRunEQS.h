// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LrBTTaskRunEQS.generated.h"

struct FEnvQueryResult;
class UEnvQuery;

/**
 * 运行 EQS 查询任务
 * 异步执行指定的环境查询（EQS），将评分最高的结果位置写入黑板
 *
 * 工作流程：
 * 1. 以 AI 控制的 Pawn 作为查询者（Querier）
 * 2. 异步执行 QueryTemplate 指定的 EQS 查询
 * 3. 查询完成后，取第一个（最优）结果的位置写入 BlackboardKey
 *
 * 注意：此任务返回 InProgress，在 OnQueryFinished 中完成，属于异步 BTTaskNode
 */
UCLASS()
class LR_API ULrBTTaskRunEQS : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULrBTTaskRunEQS();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/**
	 * EQS 查询完成回调
	 * 取最优结果的位置写入黑板，然后完成行为树任务
	 */
	void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result);

public:
	/** 要执行的 EQS 查询模板（可在蓝图/行为树编辑器中指定） */
	UPROPERTY(EditAnywhere, Category = "EQS")
	TObjectPtr<UEnvQuery> QueryTemplate;

	/** 查询结果写入的黑板键名（Vector 类型，例如 "MoveLocation"） */
	UPROPERTY(EditAnywhere, Category = "EQS")
	FName BlackboardKey;

private:
	/** 缓存的 OwnerComp 指针，异步回调中用于完成任务 */
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};