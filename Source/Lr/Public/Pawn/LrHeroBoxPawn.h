// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pawn/LrHeroPawn.h"
#include "LrHeroBoxPawn.generated.h"

class UBoxComponent;
/**
 * 
 */
UCLASS()
class LR_API ALrHeroBoxPawn : public ALrHeroPawn
{
	GENERATED_BODY()
public:
	ALrHeroBoxPawn();
protected:

	/** 碰撞体（NavAgent & Mover 都依赖它） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<UBoxComponent> LrBoxComponent;
};
