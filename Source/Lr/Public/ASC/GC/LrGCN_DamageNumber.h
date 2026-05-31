// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "LrGCN_DamageNumber.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrGCN_DamageNumber : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	// ULrGCN_DamageNumber();
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};
