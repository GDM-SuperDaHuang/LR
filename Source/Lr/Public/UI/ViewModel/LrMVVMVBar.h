// Fill out your copyright notice in the Description page of Project Settings.
// 持有 UI 数据
#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "LrMVVMVBar.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrMVVMVBar : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, FieldNotify)
	float Percent = 1.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify)
	float GhostPercent = 1.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify)
	float CurrentValue = 100.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify)
	float MaxValue = 100.f;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	FText ValueText;
public:
	void SetValue(float InCurrent,float InMax);

	void Tick(float DeltaTime);

private:

	float TargetPercent = 1.f;

	float InterpSpeed = 4.f;
};
