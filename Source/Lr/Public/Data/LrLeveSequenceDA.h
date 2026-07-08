// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LrLeveSequenceDA.generated.h"

class UAnimMontage;
class ULevelSequence;

USTRUCT()
struct FLrLeveSequenceConfig
{
	GENERATED_BODY()
	
	UPROPERTY()
	UAnimMontage* ApplyCollapseMontage = nullptr;

	UPROPERTY()
	UAnimMontage* TargetCollapseMontage = nullptr;
};

USTRUCT(BlueprintType)
struct FLrCollapseMontageConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* CollapseMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	int32 PawnType = 0; // 生物类型。
};
/**
 * 
 */
UCLASS()
class LR_API ULrLeveSequenceDA : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelSequence> SequenceTemplate;
	
	UPROPERTY(EditDefaultsOnly)
	TArray<FLrCollapseMontageConfig> LrCollapseMontageConfigs;

	FLrLeveSequenceConfig FindLeveSequenceInfo(const uint16 ApplyPawnType, const uint16 TargetPawnType) const;
	
};
