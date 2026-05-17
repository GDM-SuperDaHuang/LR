// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MoverComponent.h"
#include "LrMoverComponent.generated.h"

class ULrMovementSettings;
/**
 * 
 */
UCLASS()
class LR_API ULrMoverComponent : public UMoverComponent
{
	GENERATED_BODY()

public:
	ULrMoverComponent();

	UPROPERTY(EditAnywhere, Instanced, Category = "Realistic Movement")
	TObjectPtr<ULrMovementSettings> RealisticSettings;
	// virtual void InitializeComponent() override;
public:
	// virtual void OnRegister() override;
};
