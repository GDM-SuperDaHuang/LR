// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pawn/LrHeroPawn.h"
#include "LrHeroCapsulePawn.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ALrHeroCapsulePawn : public ALrHeroPawn
{
	GENERATED_BODY()
public:
	ALrHeroCapsulePawn();

	/** 碰撞体（NavAgent & Mover 都依赖它） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<UCapsuleComponent> LrCapsuleComponent;
};
