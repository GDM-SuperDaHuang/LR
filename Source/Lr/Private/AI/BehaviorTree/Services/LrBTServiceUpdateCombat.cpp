// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Services/LrBTServiceUpdateCombat.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Lr/Lr.h"

ULrBTServiceUpdateCombat::ULrBTServiceUpdateCombat()
{
	Interval = 0.2f;
}

void ULrBTServiceUpdateCombat::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	APawn* SelfPawn = OwnerComp.GetAIOwner()->GetPawn();
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(LrBBKeys::TargetActor));

	if (!SelfPawn || !Target)
	{
		BB->SetValueAsBool(LrBBKeys::CombatState, false);
		return;
	}

	const float Distance = FVector::Distance(SelfPawn->GetActorLocation(), Target->GetActorLocation());

	BB->SetValueAsFloat(TEXT("DistanceToTarget"), Distance);
	BB->SetValueAsBool(LrBBKeys::CombatState, Distance <= AttackRange);
}