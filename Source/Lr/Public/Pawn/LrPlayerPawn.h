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
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	/** 移动更新相关 */
    // 输入生产（被Mover调用）
    void ProduceMoverInput(float DeltaTimeMS, FMoverInputCmdContext* InputContext);
	
};
