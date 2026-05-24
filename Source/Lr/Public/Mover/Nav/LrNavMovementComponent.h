// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "LrNavMovementComponent.generated.h"

class ALrEnemyPawn;
class ULrMoverComponent;
/**
 *
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrNavMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;
	// 重写停止激活移动的函数
	virtual void StopActiveMovement() override;
	// virtual FVector GetActorFeetLocation() const override;
	
	UPROPERTY()
	TObjectPtr<ALrEnemyPawn> LrEnemyPawn;
};
