// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "NormalMeleeGC.generated.h"

class UNiagaraComponent;
/**
 * 
 */
UCLASS()
class LR_API UNormalMeleeGC : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
public:
	UNormalMeleeGC();
	// 瞬间
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

	// 持续
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	
};
