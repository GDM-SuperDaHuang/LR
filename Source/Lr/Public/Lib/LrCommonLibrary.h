// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/LrCorpseConfigDA.h"
#include "Data/LrGAListDA.h"
#include "Game/LrGameModeBase.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LrCommonLibrary.generated.h"

class ULrBuffDA;
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
	static ALrGameModeBase* GetLrGameModeBase(const UObject* WorldContextObject);

	// 死亡配置
	static FLrCorpseConfig FindCorpseConfigByPawnType(const UObject* WorldContextObject, uint32 PawnType);

	// 根据标签查找技能配置
	UFUNCTION(BlueprintPure, Category = "ULrCommonLibrary|DA", meta=(DefaultToSelf = "WorldContextObject"))
	static const FLrGAConfig& FindGAByTag(const UObject* WorldContextObject, const FGameplayTag& GATag);

	// UFUNCTION(BlueprintPure, Category = "ULrCommonLibrary|DA", meta=(DefaultToSelf = "WorldContextObject"))
	static const FPawnTypeGAConfig& FindPawnTypeGAConfig(const UObject* WorldContextObject, const uint16 PawnType);

	// 根据标签查找特效NS
	UFUNCTION(BlueprintPure, Category = "ULrCommonLibrary|DA", meta=(DefaultToSelf = "WorldContextObject"))
	static FLrNSConfig FindNSByTag(const UObject* WorldContextObject, const FGameplayTag& GATag);

	// 根据ID查找武器信息u
	UFUNCTION(BlueprintPure, Category = "ULrCommonLibrary|DA", meta=(DefaultToSelf = "WorldContextObject"))
	static FLrWeaponConfig FindWeaponByID(const UObject* WorldContextObject, const int32 WeaponID);

	// 根据GE配置
	UFUNCTION(BlueprintPure, Category = "ULrCommonLibrary|GE", meta=(DefaultToSelf = "WorldContextObject"))
	static const ULrBuffDA* GetGEDA(const UObject* WorldContextObject);

	// 在 Forward 方向周围均匀生成旋转数组（当前只返回一个）
	UFUNCTION(BlueprintPure, Category = "ULrCommonLibrary|GA", meta=(DefaultToSelf = "WorldContextObject"))
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators);

	// 打印信息
	static void PrintLog(const UObject* WorldContextObject, const APlayerController* PC);
};
