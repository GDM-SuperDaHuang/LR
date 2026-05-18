// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/Context/LrEQSContextTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Lr/Lr.h"

void ULrEQSContextTarget::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

	AAIController* AIController = Cast<AAIController>(QueryInstance.Owner.Get());
	if (!AIController)
	{
		return;
	}

	UBlackboardComponent* BB = AIController->GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(LrBBKeys::TargetActor));
	if (!Target)
	{
		return;
	}

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, Target);
}