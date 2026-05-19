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

	const FVector SelfLocation = Enemy->GetActorLocation();

	const FVector TargetLocation = Target->GetActorLocation();

	const float Distance = FVector::Distance( SelfLocation, TargetLocation);

	ULrAIStateComponent* AIState = Enemy->GetAIStateComponent();

	if (!AIState)
	{
		return EBTNodeResult::Failed;
	}

	// 进入攻击状态
	if (Distance <= AttackRange)
	{
		AIState->SetAIState( ELrAIState::Attack);

		BB->SetValueAsEnum( LrBBKeys::AIState, (uint8)ELrAIState::Attack);

		return EBTNodeResult::Succeeded;
	}

	// 目标太远
	if (Distance >= LoseTargetDistance)
	{
		BB->ClearValue( LrBBKeys::TargetActor);

		AIState->SetAIState( ELrAIState::ReturnHome);

		BB->SetValueAsEnum( LrBBKeys::AIState, (uint8)ELrAIState::ReturnHome);

		return EBTNodeResult::Succeeded;
	}

	// 持续追击
	AIState->SetAIState( ELrAIState::Chase);

	BB->SetValueAsEnum( LrBBKeys::AIState, (uint8)ELrAIState::Chase);

	return EBTNodeResult::Succeeded;
}
