// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ASC/GE/LrGEContext.h"
#include "LrExecCalcDamage.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrExecCalcDamage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	ULrExecCalcDamage();
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};
