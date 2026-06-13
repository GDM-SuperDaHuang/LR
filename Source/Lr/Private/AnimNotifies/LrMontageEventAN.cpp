// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifies/LrMontageEventAN.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Pawn/LrEnemyPawn.h"

void ULrMontageEventAN::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;
	const FAnimNotifyEvent* AnimNotifyEvent = EventReference.GetNotify();
	if (!AnimNotifyEvent) return;
	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = Owner;

	if (ALrEnemyPawn* Enemy = Cast<ALrEnemyPawn>(MeshComp->GetOwner()))
	{
		// Enemy->OnDeathRagdoll();
	}

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		EventTag,
		EventData
	);
}
