// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "UObject/Object.h"
#include "LrMovementSettings.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrMovementSettings : public UObject, public IMovementSettingsInterface
{
	GENERATED_BODY()
public:
	// ULrMovementSettings();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float Mass = 80.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air")
	float GravityScale = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float JumpImpulseForce = 1600.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air")
	float AirDragCoef = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air")
	float AirControlForce = 500.0f;

	virtual FString GetDisplayName() const override { return TEXT("Realistic Physics"); }
};
