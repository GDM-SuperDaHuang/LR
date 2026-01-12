// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "LrAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	FVector VelocityWS = FVector::ZeroVector;
};
