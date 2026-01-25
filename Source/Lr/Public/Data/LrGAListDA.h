// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "LrGAListDA.generated.h"


class UGameplayAbility;

USTRUCT(BlueprintType)
struct FLrDAConfig
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> GAClass; //技能
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag GATag = FGameplayTag(); // 技能本身

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag(); // 触发技的标签。
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ImpactSound = nullptr;
};

/**
 * 
 */
UCLASS()
class LR_API ULrGAListDA : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FLrDAConfig> GAConfigList;

	const FLrDAConfig* FindGAByTag(const FGameplayTag& GATag) const;
};
