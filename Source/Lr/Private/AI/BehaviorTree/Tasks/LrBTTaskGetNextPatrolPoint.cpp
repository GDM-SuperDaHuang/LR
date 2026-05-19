// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Tasks/LrBTTaskGetNextPatrolPoint.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/LrPatrolRouteComponent.h"
#include "Lr/Lr.h"
#include "Pawn/LrEnemyPawn.h"

ULrBTTaskGetNextPatrolPoint::ULrBTTaskGetNextPatrolPoint()
{
	NodeName = TEXT("GetNextPatrolPoint");
}

EBTNodeResult::Type ULrBTTaskGetNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ALrEnemyPawn* Enemy = Cast<ALrEnemyPawn>( OwnerComp.GetAIOwner()->GetPawn());

	if (!Enemy)
	{
		return EBTNodeResult::Failed;
	}

	ULrPatrolRouteComponent* Patrol = Enemy->GetPatrolRoute();

	if (!Patrol)
	{
		return EBTNodeResult::Failed;
	}

	const FVector Location = Patrol->GetNextPatrolPoint();

	OwnerComp.GetBlackboardComponent()->SetValueAsVector(LrBBKeys::MoveLocation,Location);

	return EBTNodeResult::Succeeded;
}