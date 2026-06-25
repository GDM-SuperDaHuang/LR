// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MoverComponent.h"
#include "LrMoverComponent.generated.h"

class ULrMovementSettings;

struct FPendingLaunchData
{
	FVector Impulse;
	float Duration = 0.3f;
};

/**
 * 
 */
UCLASS()
class LR_API ULrMoverComponent : public UMoverComponent
{
	GENERATED_BODY()

public:
	ULrMoverComponent();

	UPROPERTY(EditAnywhere, Instanced, Category = "Realistic Movement")
	TObjectPtr<ULrMovementSettings> RealisticSettings;

	bool bJumpInitiated = false;

	//攻击转身
	bool bIsInAttackWarp = false;
	FRotator AttackWarpRotation = FRotator::ZeroRotator;

	const FPendingLaunchData* GetPendingLaunchData();
	void Launch(FVector Impulse, float Duration);

	FPendingLaunchData PendingLaunchData;
};
