// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "LrGABase.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrGABase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/**
	 * 技能关联的输入标签（如"InputTag.LMB"对应左键，"InputTag.1"对应数字键1）
	 * 在编辑器中配置，用于在技能添加时绑定到输入事件
	 * 与UAureAbilitySystemComponent中的输入处理逻辑联动
	 */
	// UPROPERTY(EditDefaultsOnly, Category = "Input")
	// FGameplayTag InputTag;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;


	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;
	
	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	TSubclassOf<UGameplayEffect> DefaultCooldownGE;


	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	float CooldownDuration = 1;

	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	FGameplayTagContainer CooldownTags;
};
