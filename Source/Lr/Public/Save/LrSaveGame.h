// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "LrSaveGame.generated.h"
enum class EGAInputID : uint8;

USTRUCT(BlueprintType)
struct FLrPlayerKeyMapping
{
	GENERATED_BODY()

	// UPROPERTY(EditAnywhere)
	// FGameplayTag InputTag;

	UPROPERTY(EditAnywhere)
	EGAInputID InputId;

	UPROPERTY(EditAnywhere)
	TArray<FKey> CustomKeys;
};
/**
 * 
 */
UCLASS()
class LR_API ULrSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:

	UPROPERTY()
	TArray<FLrPlayerKeyMapping> KeyMappings;//历史记录
	
};
