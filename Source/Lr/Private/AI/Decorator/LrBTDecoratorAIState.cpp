// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorator/LrBTDecoratorAIState.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Lr/Lr.h"

ULrBTDecoratorAIState::ULrBTDecoratorAIState()
{
	NodeName = TEXT("LrBTDecorator AI State");
}

bool ULrBTDecoratorAIState::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return false;
	}
	const uint8 State = BB->GetValueAsEnum( LrBBKeys::AIState);
	return State == (uint8)RequiredState;
}
