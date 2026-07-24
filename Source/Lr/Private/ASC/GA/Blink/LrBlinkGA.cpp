// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GA/Blink/LrBlinkGA.h"

#include "AbilitySystemComponent.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/LrAllModes.h"
#include "Pawn/LrPawnBase.h"

ULrBlinkGA::ULrBlinkGA()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void ULrBlinkGA::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UE_LOG(LogTemp, Warning, TEXT("======ActivateAbility ULrBlinkGA"));

	bool bCanActivate = CanActivateAbility(Handle, ActorInfo);

	// UE_LOG(LogTemp, Warning, TEXT("CanActivate=%d"), bCanActivate);
	// ========== 0. 合法性校验 ==========
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	FGameplayTagContainer OwnedTags;
	GetAbilitySystemComponentFromActorInfo()->GetOwnedGameplayTags(OwnedTags);
	UE_LOG(LogTemp, Warning, TEXT("Owned Tags : %s"), *OwnedTags.ToStringSimple());

	ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(ActorInfo->AvatarActor.Get());
	if (!OwnerPawn)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	OwnerPawn->LrMoverComponent->QueueNextMode(LrAllModes::Blink);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void ULrBlinkGA::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
