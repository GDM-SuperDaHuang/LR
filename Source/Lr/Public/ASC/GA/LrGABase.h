// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "LrGABase.generated.h"

class UTexture2D;


USTRUCT(BlueprintType)
struct FLrCooldownConfig
{
	GENERATED_BODY()

	/** 技能唯一冷却Tag */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CooldownTag;

	/** 冷却时间 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CooldownDuration = 0.f;
};

USTRUCT(BlueprintType)
struct FLrAbilityCostConfig
{
	GENERATED_BODY()

	/** 蓝量消耗，对应 AttributeSet 里的 MP。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Mana = 0.f;

	/** 对应 AttributeSet 里的 Endurance。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Stamina = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float HP = 0.f;
};

USTRUCT(BlueprintType)
struct FLrAbilityConfig
{
	GENERATED_BODY()

	/** 技能唯一Tag */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag;

	/** 技能自己的冷却配置。多个技能可复用同一个 GE，但依靠不同 Tag 区分冷却。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLrCooldownConfig Cooldown;

	/** 技能释放时需要扣除的资源。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLrAbilityCostConfig Cost;

	/** 技能图标。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTexture2D> SkillIcon;
};

/**
 * 
 */
UCLASS()
class LR_API ULrGABase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/** 自定义冷却检测：共享 GE 时按 EffectTag 查询当前技能是否仍在冷却。 */
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	/** 使用共享冷却 GE，并在运行时注入当前技能自己的冷却时长和冷却 Tag。 */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	/** 在 Commit 前检查当前技能配置的资源是否足够。 */
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	/** 在 Commit 时直接扣除当前技能配置的资源。 */
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	/** 返回当前技能的冷却标识，供查询和调试复用。 */
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;

	/** 给 UI/蓝图用，按 AvatarActor 查询这个技能当前还剩多少冷却时间。 */
	UFUNCTION(BlueprintPure, Category="Ability|Cooldown")
	bool GetCooldownRemainingForActor(AActor* AvatarActor, float& OutRemainingTime, float& OutTotalDuration) const;

	/** 给 UI/蓝图用，快速判断这个技能当前是否处于冷却中。 */
	UFUNCTION(BlueprintPure, Category="Ability|Cooldown")
	bool IsAbilityInCooldownForActor(AActor* AvatarActor) const;

	/** 返回配置表里填写的总冷却时长，常用于 UI 初始显示。 */
	UFUNCTION(BlueprintPure, Category="Ability|Config")
	float GetConfiguredCooldownDuration() const;

	/** 所有技能共用的冷却 GE 模板，真正的 Tag/时长在运行时按技能注入。 */
	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	TSubclassOf<UGameplayEffect> DefaultCooldownGE;

	/** 当前技能的统一配置入口，包含技能 Tag、冷却和消耗。 */
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	FLrAbilityConfig AbilityConfig;

protected:
	/** 判断当前技能是否配置了可生效的冷却数据。 */
	bool HasValidCooldownConfig() const;
	/** 判断当前技能是否配置了任意一种资源消耗。 */
	bool HasAnyCost() const;
	/** 通用资源检测，避免 MP/耐力/HP 分别写重复逻辑。 */
	bool CheckAttributeCost(const UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute, float Cost) const;
	/** 通用资源扣减，直接作用于 ASC 的属性值。 */
	void ApplyAttributeCost(UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute, float Cost) const;
	/** 通过 ASC 上带有该技能 EffectTag 的活动 GE 查询剩余冷却和总时长。 */
	bool GetCooldownRemainingFromASC(const UAbilitySystemComponent* ASC, float& OutRemainingTime, float& OutTotalDuration) const;

	/** 运行时缓存当前技能的冷却 Tag 容器，避免每次都临时构造返回对象。 */
	mutable FGameplayTagContainer CachedCooldownTags;
};
