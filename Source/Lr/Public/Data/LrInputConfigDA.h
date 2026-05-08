// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "LrInputConfigDA.generated.h"


class UInputMappingContext;

// 默认按键绑定
USTRUCT(BlueprintType)
struct FLrDefaultBindInputFKey
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr;

	// 操作标识（例如 "Action.Jump"）
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;

	// 新增：该动作绑定的具体按键（可多个）
	UPROPERTY(EditDefaultsOnly)
	TArray<FKey> BoundKeys;
};

// // 玩家自定义按键绑定
// USTRUCT(BlueprintType)
// struct FLrPlayerBindInputFKey
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere)
// 	FGameplayTag InputTag;
//
// 	UPROPERTY(EditAnywhere)
// 	TArray<FKey> CustomKeys;
// };


// USTRUCT(BlueprintType)
// struct FLrInputAction
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditDefaultsOnly)
// 	const class UInputAction* InputAction = nullptr;
//
// 	UPROPERTY(EditDefaultsOnly)
// 	FGameplayTag InputTag = FGameplayTag();
// };

/**
 * 
 */
UCLASS()
class LR_API ULrInputConfigDA : public UDataAsset
{
	GENERATED_BODY()

public:
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const;
	void InitBindInputFKey(UInputMappingContext* IMC) const;


	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	// TArray<FLrInputAction> AbilityInputActions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FLrDefaultBindInputFKey> LrBindInputFKeyList;
};
