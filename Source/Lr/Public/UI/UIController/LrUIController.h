// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LrUIController.generated.h"

class ULrASComponent;
class ULrMVVMVBar;
/**
 * 
 */
UCLASS()
class LR_API ULrUIController : public UObject
{
	GENERATED_BODY()
public:

	void Init(ULrASComponent* InASComponent);

	void Tick(float DeltaTime);

public:

	UPROPERTY()
	TObjectPtr<ULrMVVMVBar> HPVM;

	UPROPERTY()
	TObjectPtr<ULrMVVMVBar> MPVM;

private:

	UPROPERTY()
	TObjectPtr<ULrASComponent> ASComponent;

private:

	void OnHPChanged(float Current,float Max);

	void OnMPChanged(float Current,float Max);
};
