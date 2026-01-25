// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASC/GA/LrGABase.h"
#include "LrNormalMeleeGATest.generated.h"

class ALrPawnBase;
/**
 * 
 */
UCLASS()
class LR_API ULrNormalMeleeGATest : public ULrGABase
{
	GENERATED_BODY()
public:
	// ULrNormalMeleeGA();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	void OnMontageFinished();


	UFUNCTION()
	void OnAttackEvent(FGameplayEventData Payload);
	
	void SpawnWeaponTrailFX(ALrPawnBase* OwnerPawn);
	void PerformMeleeTrace(ALrPawnBase* OwnerPawn, TArray<FHitResult>& Array);

	void OnAttackEventReceived(const FGameplayEventData* GameplayEventData) const;
};
