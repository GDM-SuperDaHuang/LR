// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASC/GA/LrGABase.h"
#include "ASC/GE/LrGEContext.h"
#include "Component/Combat/LrCombatComponentBase.h"
#include "LrLightningGA.generated.h"

class ALrLightning;
class ALrPawnBase;

/**
 * 链式闪电技能
 */
UCLASS()
class LR_API ULrLightningGA : public ULrGABase
{
	GENERATED_BODY()
	

public:
	ULrLightningGA();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	UFUNCTION()
	void OnMontageFinished();

	/** Montage 伤害触发帧回调 */
	UFUNCTION()
	void OnAttackEvent(FGameplayEventData Payload);
	
	
};