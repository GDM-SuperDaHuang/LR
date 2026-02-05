// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "LrButtonInfo.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FLrButtonInfoClicked,
	ULrButtonInfo*, Button
);
/**
 * 
 */
UCLASS()
class LR_API ULrButtonInfo : public UButton
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button")
	int32 ButtonID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button")
	int32 WeaponID = -1;
	
	// 1:穿上装备
	// 2:脱下装备
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Button")
	int32 BehaviorID = -1;


	/** 对外统一的点击事件 */
	UPROPERTY(BlueprintAssignable, Category="Button")
	FLrButtonInfoClicked OnButtonInfoClicked;
	
protected:
	virtual void SynchronizeProperties() override
	{
		Super::SynchronizeProperties();

		OnClicked.Clear();
		OnClicked.AddDynamic(this, &ULrButtonInfo::HandleClicked);
	}

	UFUNCTION()
	void HandleClicked()
	{
		OnButtonInfoClicked.Broadcast(this);
	}
	
};
