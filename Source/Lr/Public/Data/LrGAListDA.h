// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "LrGAListDA.generated.h"


USTRUCT(BlueprintType)
struct FLrDAConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag GATag = FGameplayTag();

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
