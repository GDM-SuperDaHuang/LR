// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LrAIControllerBase.h"
#include "LrNormalAIController.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ALrNormalAIController : public ALrAIControllerBase
{
	GENERATED_BODY()
protected:

	virtual void InitializeAI() override;

	
};
