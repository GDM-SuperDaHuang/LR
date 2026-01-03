// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MoverSimulationTypes.h"
#include "Pawn/LrPawnBase.h"
#include "LrPlayerPawn.generated.h"

class UMoverComponent;

// 自定义输入结构体
// USTRUCT()
// struct FMyMoverInput : public FMoverInputCmdContext
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY()
// 	FVector2D MoveDirection;
//
// 	UPROPERTY()
// 	bool bJump;
//
// 	UPROPERTY()
// 	bool bDash;
// };
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
	UPROPERTY(VisibleAnywhere)
	UMoverComponent* MoverComponent;
	
};
