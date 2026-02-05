// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/LrGAListDA.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LrCommonLibrary.generated.h"

struct FLrNSConfig;
class UAbilitySystemComponent;
struct FGameplayTag;
struct FLrGAConfig;
/**
 * 
 */
UCLASS()
class LR_API ULrCommonLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = Ability, Meta=(DefaultToSelf = "Actor"))
	static UAbilitySystemComponent* GetASC(AActor* Actor);

	// 根据标签查找技能配置
	UFUNCTION(BlueprintPure, Category = "ULrCommonLibrary|DA", meta=(DefaultToSelf = "WorldContextObject"))
	static const FLrGAConfig& FindGAByTag(const UObject* WorldContextObject, const FGameplayTag& GATag);

	// 根据标签查找特效NS
	UFUNCTION(BlueprintPure, Category = "ULrCommonLibrary|DA", meta=(DefaultToSelf = "WorldContextObject"))
	static FLrNSConfig FindNSByTag(const UObject* WorldContextObject, const FGameplayTag& GATag);

	// 根据ID查找武器信息u
	UFUNCTION(BlueprintPure, Category = "ULrCommonLibrary|DA", meta=(DefaultToSelf = "WorldContextObject"))
	static FLrWeaponConfig FindWeaponByID(const UObject* WorldContextObject, const int32 WeaponID);


	// 打印信息
	static void PrintLog(const UObject* WorldContextObject,const APlayerController* PC);
};
