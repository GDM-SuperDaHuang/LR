// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "LrWorldWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrWorldWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
public:
	ULrWorldWidgetComponent();

	// UPROPERTY(EditDefaultsOnly, Category="UI")
	// TSubclassOf<UUserWidget> LrWidgetClass;
};
