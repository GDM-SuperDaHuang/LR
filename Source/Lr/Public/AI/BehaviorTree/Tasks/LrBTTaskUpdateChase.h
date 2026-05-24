// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LrBTTaskUpdateChase.generated.h"

/**
 * 追踪
 */
UCLASS()
class LR_API ULrBTTaskUpdateChase : public UBTTaskNode
{
	GENERATED_BODY()
public:

	ULrBTTaskUpdateChase();

	virtual EBTNodeResult::Type ExecuteTask( UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
