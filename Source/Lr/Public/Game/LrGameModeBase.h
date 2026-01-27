// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LrGameModeBase.generated.h"

class ULrGAListDA;
/**
 * 
 */
UCLASS()
class LR_API ALrGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<ULrGAListDA> LrGAConfigList;
	
};
