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
	CachedOwnerComp = &OwnerComp;

	APawn* Pawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!Pawn || !QueryTemplate)
	{
		return EBTNodeResult::Failed;
	}

	FEnvQueryRequest QueryRequest(QueryTemplate, Pawn);
	QueryRequest.Execute(EEnvQueryRunMode::SingleResult, this, &ULrBTTaskRunEQS::OnQueryFinished);

	return EBTNodeResult::InProgress;
}

void ULrBTTaskRunEQS::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
	if (!CachedOwnerComp)
	{
		return;
	}

	if (!Result->IsSuccessful())
	{
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);
		return;
	}

	const FVector Location = Result->GetItemAsLocation(0);
	CachedOwnerComp->GetBlackboardComponent()->SetValueAsVector(BlackboardKey, Location);

	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}