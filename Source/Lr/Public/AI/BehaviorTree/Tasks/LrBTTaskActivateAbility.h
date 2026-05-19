// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LrBTTaskActivateAbility.generated.h"

/**
 * 激活技能任务
 * 在行为树中激活指定 GameplayTag 对应的 GAS 技能
 *
 * 工作流程：
 * 1. 从 OwnerComp 获取 AI 控制的 Pawn
 * 2. 通过 AbilitySystemBlueprintLibrary 获取 Pawn 上的 ASC
 * 3. 调用 TryActivateAbilitiesByTag 激活 AbilityTag 对应的技能
 */
UCLASS()
class LR_API ULrBTTaskActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULrBTTaskActivateAbility();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** 要激活的技能 GameplayTag（例如 "Ability.Attack.Melee"） */
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag AbilityTag;
	bool bWaitingAbilityEnd = false;
};