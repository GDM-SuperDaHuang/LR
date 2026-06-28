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

	// void ApplyDamageToTarget(AActor* Target, const FHitResult& Hit);
	void ApplyDamageToTarget(AActor* Target, FDamageEffectParams DamageEffectParams) const;
};
