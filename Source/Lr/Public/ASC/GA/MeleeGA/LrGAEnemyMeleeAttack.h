// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASC/GA/LrGABase.h"
#include "ASC/GE/LrGEContext.h"
#include "Component/Combat/LrCombatComponentBase.h"
#include "LrGAEnemyMeleeAttack.generated.h"

/**
 * 普通近战攻击
 */
UCLASS()
class LR_API ULrGAEnemyMeleeAttack : public ULrGABase
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnAttackEvent(FGameplayEventData Payload);

	// 被打断
	UFUNCTION()
	void OnMontageCancelled();

	//正常结束
	UFUNCTION()
	void OnMontageCompleted();

	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn = true))
	FDamageEffectParams DamageEffectParams;

	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn = true))
	FLrCombatQueryParams ConeParams;

	// 单体类型
	UPROPERTY()
	TWeakObjectPtr<AActor> TargetAActor; //目标可能死亡
};
