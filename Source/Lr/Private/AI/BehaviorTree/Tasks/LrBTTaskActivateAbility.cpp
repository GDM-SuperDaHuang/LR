// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Tasks/LrBTTaskActivateAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"

ULrBTTaskActivateAbility::ULrBTTaskActivateAbility()
{
	NodeName = "Activate Ability";
	bNotifyTick = true;
}

EBTNodeResult::Type ULrBTTaskActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 获取 AI 当前控制的 Pawn cast?
	APawn* Pawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	// 通过 GAS 蓝图库获取 Pawn 上的 AbilitySystemComponent
	// 不直接 Cast 是因为 ASC 可能在 PlayerState 或 Pawn 上，蓝图库会统一查找
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	// 尝试激活所有匹配 AbilityTag 的技能
	// 如果有多个技能共享同一标签，它们都会被尝试激活
	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
	if (!bActivated)
	{
		return EBTNodeResult::Failed;
	}
	bWaitingAbilityEnd = true;
	return EBTNodeResult::Succeeded;
}

void ULrBTTaskActivateAbility::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	if (!bWaitingAbilityEnd)
	{
		FinishLatentTask( OwnerComp, EBTNodeResult::Succeeded);
	}
}
