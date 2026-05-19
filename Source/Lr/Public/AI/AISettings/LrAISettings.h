// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LrAISettings.generated.h"

class ALrAIControllerBase;
/**
 * 
 */
UCLASS(Config=Game, DefaultConfig)
class LR_API ULrAISettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	// 普通敌人 Controller
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="AI")
	TSoftClassPtr<ALrAIControllerBase> NormalAIControllerClass;

	// Boss Controller
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="AI")
	TSoftClassPtr<ALrAIControllerBase> BossAIControllerClass;	
};
