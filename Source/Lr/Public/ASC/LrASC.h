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
	void AddGA(const TArray<TSubclassOf<UGameplayAbility>>& GAList);
};
