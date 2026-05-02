// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "LrCharacterMoverComponent.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrCharacterMoverComponent : public UCharacterMoverComponent
{
	GENERATED_BODY()

public:
	ULrCharacterMoverComponent();
	// Methods.
	virtual void OnRegister() override;
	
};
