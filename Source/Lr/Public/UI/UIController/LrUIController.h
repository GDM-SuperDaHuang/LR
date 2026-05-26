// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LrUIController.generated.h"

struct FGameplayTag;
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

public:
	void OnASChanged(FGameplayTag ASTag, float Current, float Max);


	UPROPERTY()
	TObjectPtr<ULrMVVMVBar> HPVM;

	UPROPERTY()
	TObjectPtr<ULrMVVMVBar> MPVM;

private:
	// UPROPERTY()
	// TObjectPtr<ULrASComponent> ASComponent;

private:
	void OnHPChanged(float Current, float Max);

	void OnMPChanged(float Current, float Max);
};
