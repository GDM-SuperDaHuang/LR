// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MoverSimulationTypes.h"
#include "Pawn/LrPawnBase.h"
#include "LrPlayerPawn.generated.h"

class UMoverComponent;
/**
 * 
 */
UCLASS()
class LR_API ALrPlayerPawn : public ALrPawnBase
{
	GENERATED_BODY()
	
public:
	ALrPlayerPawn();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:

	
};
