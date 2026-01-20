// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LrCommonLibrary.generated.h"

struct FGameplayTag;
struct FLrDAConfig;
/**
 * 
 */
UCLASS()
class LR_API ULrCommonLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	// 查找技能配置
	UFUNCTION(BlueprintPure, Category = "ULrCommonLibrary|DA", meta=(DefaultToSelf = "WorldContextObject"))
	static const FLrDAConfig* FindGAByTag(const UObject* WorldContextObject, const FGameplayTag& GATag);
};
