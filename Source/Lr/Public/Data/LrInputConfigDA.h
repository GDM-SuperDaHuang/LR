// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "LrInputConfigDA.generated.h"


enum class EGAInputID : uint8;

class UInputMappingContext;

// 默认 技能按键 绑定，没有wasd移动
USTRUCT(BlueprintType)
struct FLrDefaultBindInputFKey
{
	GENERATED_BODY()
	//输入事件
	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr;

	// 操作标识（例如 "Action.Jump"）
	// UPROPERTY(EditDefaultsOnly)
	// FGameplayTag InputTag;

	// 操作标识（例如 "Action.Jump"）
	UPROPERTY(EditDefaultsOnly)
	EGAInputID InputId;

	// 新增：该动作绑定的具体按键（可多个）
	UPROPERTY(EditDefaultsOnly)
	TArray<FKey> BoundKeys;
};

USTRUCT(BlueprintType)
struct FLrAxisKey
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FKey BoundKey;

	UPROPERTY(EditDefaultsOnly)
	float Scale = 1.f; // +1 or -1

	UPROPERTY(EditDefaultsOnly)
	bool IsFU = true; // true:上下， fasle:左右
};

// 移动按键
USTRUCT(BlueprintType)
struct FLrAxisBindInputFKey
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	const UInputAction* InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TArray<FLrAxisKey> LrAxisKeyList;
};

/**
 * 
 */
UCLASS()
class LR_API ULrInputConfigDA : public UDataAsset
{
	GENERATED_BODY()

public:
	// const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FLrAxisBindInputFKey> LrAxisBindInputFKeyList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FLrDefaultBindInputFKey> LrBindInputFKeyList;

	UPROPERTY(EditDefaultsOnly)
	const UInputAction* LookAction = nullptr;

};
