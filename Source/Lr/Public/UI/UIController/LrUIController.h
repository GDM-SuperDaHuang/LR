// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LrUIController.generated.h"

class ULrMVVMSkillSlot;
class ALrPawnBase;
struct FGameplayTag;
class APawn;
class ULrASC;
class ULrMVVMVBar;

/**
 * 
 */
UCLASS()
class LR_API ULrUIController : public UObject
{
	GENERATED_BODY()

public:
	void Init();

	void Tick(float DeltaTime);

	void InitSkillSlots(ALrPawnBase* OwnerPawn);

public:
	void OnASChanged(FGameplayTag ASTag, float Current, float Max);


	UPROPERTY()
	TObjectPtr<ULrMVVMVBar> HPVM;

	UPROPERTY()
	TObjectPtr<ULrMVVMVBar> MPVM;

	UPROPERTY()
	TArray<TObjectPtr<ULrMVVMSkillSlot>> SkillSlotVMs;

private:
	// UPROPERTY()
	// TObjectPtr<ULrASComponent> ASComponent;

private:
	void OnHPChanged(float Current, float Max);

	void OnMPChanged(float Current, float Max);
};
