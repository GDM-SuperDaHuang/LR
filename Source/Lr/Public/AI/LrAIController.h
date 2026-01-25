// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "LrAIController.generated.h"

class UBehaviorTreeComponent;
/**
 * 
 */
UCLASS()
class LR_API ALrAIController : public AAIController
{
	GENERATED_BODY()

public:
	ALrAIController();

protected:
	// virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BTComponent;
};
