// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Tasks/LrBTTaskActivateAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/LrCombatComponent.h"
#include "Data/LrBuffDA.h"
#include "Lib/LrCommonLibrary.h"
#include "Lr/Lr.h"

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

	ULrCombatComponent* Combat = Pawn->FindComponentByClass<ULrCombatComponent>();
	if (!Combat)
	{
		return EBTNodeResult::Failed;
	}

	//是否可以攻击
	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(LrBBKeys::TargetActor));
	if (!Combat->CanAttack(Target))
	{
		return EBTNodeResult::Failed;
	}

	//是否开始攻击
	if (!Combat->StartAttack())
	{
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Succeeded;
}

void ULrBTTaskActivateAbility::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void ULrBTTaskActivateAbility::ApplyDamage(AActor* Target)
{
	if (!Target) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC) return;

	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	Context.AddSourceObject(this);
	
	const ULrBuffDA* LrBuffDA = ULrCommonLibrary::GetGEDA(this);
	// 创建GE
	FGameplayEffectSpecHandle Spec = TargetASC->MakeOutgoingSpec(LrBuffDA->MeleeAttackEffectClass, 1.0f, Context);

	if (Spec.IsValid())
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}
