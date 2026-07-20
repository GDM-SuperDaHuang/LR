// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GE/LrGEContext.h"
#include "LrASC.generated.h"

class ALrPawnBase;
/**
 * 
 */
UCLASS()
class LR_API ULrASC : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// 初始化 技能
	// void AddGA(const TArray<FGameplayTag>& GATagList);

	// 初始化 所有的技能
	void AddAllGA(ALrPawnBase* PawnBase);
	/**************************************** 按钮 技能触发相关 相关 ************************/
	// void AbilityInputTagPressed(const FGameplayTag& InputTags);
	// void AbilityInputTagHeld(const FGameplayTag& InputTags);
	// void AbilityInputTagReleased(const FGameplayTag& InputTags);

	void AbilityInputTagPressed0(const int32 InputId);
	void AbilityInputTagHeld0(const int32& InputId);
	void AbilityInputTagReleased0(const int32& InputId);
	/**************************************** 按钮 技能触发相关 相关 ************************/

	/** 给 UI/蓝图按冷却 Tag 查询剩余冷却时间，不必持有具体技能对象。 */
	bool GetCooldownRemainingByTag(FGameplayTag CooldownTag, float& OutRemainingTime, float& OutTotalDuration) const;

	/** 给 UI/蓝图快速判断某个冷却 Tag 当前是否生效。 */
	bool IsCooldownActiveByTag(FGameplayTag CooldownTag) const;

	//
	void ApplyDamageToTarget(AActor* SourceActor, AActor* Target, FDamageEffectParams DamageEffectParams) const;
};
