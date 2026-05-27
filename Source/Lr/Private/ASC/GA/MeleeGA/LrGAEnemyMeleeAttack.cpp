// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GA/MeleeGA/LrGAEnemyMeleeAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "ASC/LrASC.h"
#include "Lib/LrCommonLibrary.h"
#include "Pawn/LrPawnBase.h"


void ULrGAEnemyMeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(ActorInfo->AvatarActor.Get());
	if (!OwnerPawn)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	//选择一个Montage
	FLrGAConfig LrDAConfig = ULrCommonLibrary::FindGAByTag(OwnerPawn, GetAssetTags().First());
	TArray<UAnimMontage*> AnimMontages = LrDAConfig.MontageList;
	int32 Length = AnimMontages.Num();
	if (Length <= 0)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	int32 RandomIndex = FMath::RandRange(0, Length - 1);
	UAnimMontage* MeleeMontage = AnimMontages[RandomIndex];

	// 播放 Montage
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MeleeMontage,
		1.f);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Montage 正常结束
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);

	// Montage 被打断
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

void ULrGAEnemyMeleeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!Pawn)
	{
		return;
	}
	ULrCombatComponentBase* Combat = Pawn->FindComponentByClass<ULrCombatComponentBase>();
	if (!Combat)
	{
		return;
	}

	// Combat->FinishAttack();
}

void ULrGAEnemyMeleeAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void ULrGAEnemyMeleeAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	if (ULrASC* LrASC = Cast<ULrASC>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		if (ULrCombatComponentBase* Combat = GetAvatarActorFromActorInfo()->FindComponentByClass<ULrCombatComponentBase>())
		{
			ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(GetAvatarActorFromActorInfo());
			
			ConeParams.Origin = OwnerPawn->GetActorLocation();
			AActor* TargetAActor = Combat->GetClosestEnemyInCone(ConeParams);
			LrASC->ApplyDamageToTarget(TargetAActor, DamageEffectParams);
		}
	}
}
