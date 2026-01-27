// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "LrASC.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrASC : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// 初始化 技能
	void AddGA(const TArray<FGameplayTag>& GATagList);

	/**************************************** 按钮 技能触发相关 相关 ************************/
	void AbilityInputTagPressed(const FGameplayTag& InputTags);
	void AbilityInputTagHeld(const FGameplayTag& InputTags);
	void AbilityInputTagReleased(const FGameplayTag& InputTags);
	/**************************************** 按钮 技能触发相关 相关 ************************/

	void ApplyDamageToTarget(AActor* Target, const FHitResult& Hit);
};
