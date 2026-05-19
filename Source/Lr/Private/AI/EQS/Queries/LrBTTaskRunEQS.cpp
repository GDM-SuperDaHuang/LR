// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/Queries/LrBTTaskRunEQS.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"

ULrBTTaskRunEQS::ULrBTTaskRunEQS()
{
	NodeName = "Run EQS";
}

EBTNodeResult::Type ULrBTTaskRunEQS::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 缓存 OwnerComp，供异步回调使用
	// 行为树任务执行完后 OwnerComp 可能失效，必须提前保存
	CachedOwnerComp = &OwnerComp;

	APawn* Pawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!Pawn || !QueryTemplate)
	{
		return EBTNodeResult::Failed;
	}

	// 创建 EQS 查询请求，以 AI 控制的 Pawn 作为查询者
	// SingleResult 模式：只返回评分最高的一个结果，减少计算开销
	FEnvQueryRequest QueryRequest(QueryTemplate, Pawn);
	QueryRequest.Execute(EEnvQueryRunMode::SingleResult, this, &ULrBTTaskRunEQS::OnQueryFinished);

	// 返回 InProgress：行为树等待 OnQueryFinished 中的 FinishLatentTask
	return EBTNodeResult::InProgress;
}

void ULrBTTaskRunEQS::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
	if (!CachedOwnerComp)
	{
		return;
	}

	// 查询失败（无有效结果）时以 Failed 完成行为树任务
	if (!Result->IsSuccessful())
	{
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 取第一个（最优）结果的位置坐标写入黑板
	// 行为树的后续 MoveTo 任务可读取此位置进行移动
	const FVector Location = Result->GetItemAsLocation(0);
	CachedOwnerComp->GetBlackboardComponent()->SetValueAsVector(BlackboardKey, Location);

	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}