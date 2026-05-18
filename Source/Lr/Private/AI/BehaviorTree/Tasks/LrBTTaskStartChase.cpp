// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Tasks/LrBTTaskStartChase.h"

#include "NavigationSystem.h"
#include "AI/LrAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Lr/Lr.h"

ULrBTTaskStartChase::ULrBTTaskStartChase()
{
	NodeName = TEXT("Start Chase");
}

EBTNodeResult::Type ULrBTTaskStartChase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* Pawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
	{
		return EBTNodeResult::Failed;
	}

	FNavLocation RandomPoint;
	const bool bFound = NavSystem->GetRandomReachablePointInRadius(
		Pawn->GetActorLocation(),
		SearchRadius,
		RandomPoint);

	if (!bFound)
	{
		return EBTNodeResult::Failed;
	}

	OwnerComp.GetBlackboardComponent()->SetValueAsVector(LrBBKeys::MoveLocation, RandomPoint.Location);

	return EBTNodeResult::Succeeded;
}