// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Component/LrAnimationComponent.h"
#include "LrAnimInstance.generated.h"


class ULrAnimationComponent;
class ULrMoverComponent;
/**
 * 动画模式，状态管理
 */
UCLASS()
class LR_API ULrAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UPROPERTY(BlueprintReadOnly, Category="Anim")
	FLrAnimMovementData MovementData;

	
	UPROPERTY(BlueprintReadOnly)
	float Speed = 0.f; //速度大小

	UPROPERTY(BlueprintReadOnly)
	float Speed2 = 0.f; //速度大小

	
	UPROPERTY(Transient)
	TObjectPtr<ULrAnimationComponent> AnimationComponent;
};
