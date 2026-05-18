// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Tasks/LrBTTaskActivateAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"

ULrBTTaskActivateAbility::ULrBTTaskActivateAbility()
{
	NodeName = "Activate Ability";
}

EBTNodeResult::Type ULrBTTaskActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* Pawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));

	return EBTNodeResult::Succeeded;
}