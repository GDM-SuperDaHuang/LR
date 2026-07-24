// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASC/GA/LrGABase.h"
#include "ASC/GE/LrGEContext.h"
#include "Pawn/LrPawnBase.h"
#include "LrNormalMeleeGA.generated.h"

/**
 * 普通近战攻击
 */
UCLASS()
class LR_API ULrNormalMeleeGA : public ULrGABase
{
	GENERATED_BODY()

public:
	ULrNormalMeleeGA();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnMontageFinished();


	UFUNCTION()
	void OnAttackEvent(FGameplayEventData Payload);

	void SpawnWeaponTrailFX(ALrPawnBase* OwnerPawn);
	void PerformMeleeTrace(ALrPawnBase* OwnerPawn, TArray<FHitResult>& Array);

	// void OnAttackEventReceived(const FGameplayEventData* GameplayEventData) const;

	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn = true))
	FDamageEffectParams DamageEffectParams;
	
	UPROPERTY(EditDefaultsOnly)
	FLrCombatQueryParams ConeParams;

	// 单体类型
	UPROPERTY()
	TWeakObjectPtr<AActor> TargetAActor; //目标可能死亡


	UPROPERTY(EditDefaultsOnly)
	int32 TestNum = 0;

};
