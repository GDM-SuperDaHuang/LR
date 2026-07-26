// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pawn/LrEnemyPawn.h"
#include "LrEnemyCapsulePawn.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ALrEnemyCapsulePawn : public ALrEnemyPawn
{
	GENERATED_BODY()
public:
	ALrEnemyCapsulePawn();

	/** 碰撞体（NavAgent & Mover 都依赖它） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<UCapsuleComponent> LrCapsuleComponent;
};
