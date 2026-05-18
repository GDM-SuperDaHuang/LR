// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LrBTTaskActivateAbility.generated.h"

/**
 * 激活技能任务
 * 在行为树节点中激活指定 GameplayTag 对应的 GAS 技能
 */
UCLASS()
class LR_API ULrBTTaskActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULrBTTaskActivateAbility();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 要激活的技能标签 */
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag AbilityTag;
};