// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifies/LrMontageEventAN.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbilityTypes.h"

void ULrMontageEventAN::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;
	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		EventTag,
		EventData
	);

	
	// if (AController* Controller = MeshComp->GetOwner()->GetInstigatorController())
	// {
	// 	if (APawn* Pawn = Cast<APawn>(Controller->GetPawn()))
	// 	{
	// 		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn))
	// 		{
	// 			FGameplayEventData EventData ;
	// 			EventData.EventTag = EventTag;  // e.g., Montage.Attack
	// 			ASC->HandleGameplayEvent(EventTag,EventData);
	// 			// 可选：填充 TargetData（e.g., 通过 trace 获取击中）
	// 			// FHitResult Hit;  // 执行 sphere trace 从武器位置
	// 			// ... trace 逻辑 ...
	// 			// EventData.TargetData.SetHitResult(Hit);
	// 			// ASC.Handle
	// 			// ASC->HandleGameplayEvent(EventTag, EventData);
	// 		}
	// 	}
	// }
}
