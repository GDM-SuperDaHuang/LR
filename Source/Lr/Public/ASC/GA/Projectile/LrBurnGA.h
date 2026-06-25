// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASC/GA/LrGABase.h"
#include "ASC/GE/LrGEContext.h"
#include "Component/Combat/LrCombatComponentBase.h"
#include "LrBurnGA.generated.h"

class ALrProjectile;
/**
 * 
 */
UCLASS()
class LR_API ULrBurnGA : public ULrGABase
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectiles(const FVector& ProjectileTargetLocation, bool bOverridePitch, float PitchOverride, AActor* HomingTarget);

	/**
	 * 投射物的类模板（在编辑器中指定，如AFireballProjectile、IColdArrowProjectile）
	 * 技能激活时会根据此模板生成具体的投射物Actor
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ALrProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	float ProjectilesSpread = 90.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	float ConeLength = 2000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	FDamageEffectParams DamageEffectParams;

	//是否是追踪弹
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	bool bLaunchHomingProjectiles = true;
	
	UFUNCTION()
	void OnMontageFinished();
	
	UFUNCTION()
	void OnAttackEvent(FGameplayEventData Payload);


	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn = true))
	FLrCombatQueryParams ConeParams;

};
