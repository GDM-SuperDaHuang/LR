// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LrLeveSequenceDA.generated.h"

class UAnimMontage;
class ULevelSequence;
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
	UAnimMontage* CollapseMontage = nullptr;
};
