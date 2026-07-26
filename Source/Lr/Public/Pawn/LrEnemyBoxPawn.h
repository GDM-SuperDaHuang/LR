// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pawn/LrEnemyPawn.h"
#include "LrEnemyBoxPawn.generated.h"

class UBoxComponent;
/**
 * 
 */
UCLASS()
class LR_API ALrEnemyBoxPawn : public ALrEnemyPawn
{
	GENERATED_BODY()
public:
	ALrEnemyBoxPawn();
protected:

	/** 碰撞体（NavAgent & Mover 都依赖它） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<UBoxComponent> LrBoxComponent;
};
