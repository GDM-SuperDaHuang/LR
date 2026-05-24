// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Tasks/LrBTTaskUpdateChase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/LrAIStateComponent.h"
#include "Lr/Lr.h"
#include "Pawn/LrEnemyPawn.h"

ULrBTTaskUpdateChase::ULrBTTaskUpdateChase()
{
	NodeName = TEXT("Update Chase");
}

EBTNodeResult::Type ULrBTTaskUpdateChase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ALrEnemyPawn* Enemy = Cast<ALrEnemyPawn>( OwnerComp.GetAIOwner()->GetPawn());
	if (!Enemy)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>( BB->GetValueAsObject( LrBBKeys::TargetActor));
	if (!Target)
	{
		return EBTNodeResult::Failed;
	}
	
	ULrAIStateComponent* AIState = Enemy->GetAIStateComponent();
	if (!AIState)
	{
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Succeeded;
}
