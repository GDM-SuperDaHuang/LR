// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MoverSimulationTypes.h"
#include "Pawn/LrPawnBase.h"
#include "LrPlayerPawn.generated.h"

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
	virtual void PostInitializeComponents() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	// Entry point for input production. Do not override. To extend in derived character types, override OnProduceInput for native types or implement "Produce Input" blueprint event
	virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;
	// Override this function in native class to author input for the next simulation frame. Consider also calling Super method.
	virtual void OnProduceInput(float DeltaMs, FMoverInputCmdContext& InputCmdResult);

private:
	// Mover输入生产
	void ProduceMoverInput(float DeltaTimeMS, FMoverInputCmdContext* InputContext);
	
};
