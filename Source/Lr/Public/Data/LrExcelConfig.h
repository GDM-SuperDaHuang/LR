// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "LrExcelConfig.generated.h"

/**
 * 属性表
 */
USTRUCT(BlueprintType)
struct  FLrProjectileConfigRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Speed = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LifeTime = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CollisionRadius = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHoming = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HomingAcceleration = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAOE = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AOERadius = 300.f;
};

/**
 * 属性表
 */
USTRUCT(BlueprintType)
struct FLrInitASRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 列名必须和 Excel 表头完全一致

	// 第一列 Name 不需要写在这里，UE 会自动作为主键处理

	// 第二列：属性的 GameplayTag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FGameplayTag ASTag;

	// 第三列：属性的基础数值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float BaseValue = 0.f;
};

/**
 * 
 */
UCLASS()
class LR_API ULrExcelConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	UDataTable* LrASDT;

	const TArray<FLrInitASRow*> FindAllAS() const;
	
	const TArray<FLrProjectileConfigRow*> FindAllProjectileConfig() const;

};
