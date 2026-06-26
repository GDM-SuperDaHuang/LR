// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "LrGCNBurn.generated.h"

class ALrPawnBase;
class UAudioComponent;
class UNiagaraComponent;
class UNiagaraSystem;
/**
 * 
 */
UCLASS()
class LR_API ALrGCNBurn : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

	ALrGCNBurn();

protected:
	// 燃烧 Niagara 特效
	// UPROPERTY(EditDefaultsOnly, Category="Burn")
	// TObjectPtr<UNiagaraSystem> BurnNiagara;

	// 循环燃烧音效
	UPROPERTY(EditDefaultsOnly, Category="Burn")
	TObjectPtr<USoundBase> BurnLoopSound;

	// 音效组件实例
	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComponent;

	// 自身
	UPROPERTY()
	ALrPawnBase* LrPawn;

public:
	// GE Apply 时
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	// GE Remove 时
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	// Stack 改变时
	virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:
	void UpdateBurnIntensity(const FGameplayCueParameters& Parameters);
};
