// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "LrBTDecoratorAIState.generated.h"

enum class ELrAIState : uint8;
/**
 * 
 */
UCLASS(Blueprintable)
class LR_API ULrBTDecoratorAIState : public UBTDecorator
{
	GENERATED_BODY()
public:

	ULrBTDecoratorAIState();
	virtual bool CalculateRawConditionValue( UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:

	UPROPERTY(EditAnywhere)
	ELrAIState RequiredState;
};
