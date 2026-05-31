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
 * 优化：使用事件驱动代替每帧轮询，数据变化时才更新
 */
UCLASS()
class LR_API ULrAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** 委托回调：AnimationComponent 数据变化时触发 */
	void  OnMovementDataUpdated(const FLrAnimMovementData& NewData);

	UPROPERTY(BlueprintReadOnly, Category="Anim")
	FLrAnimMovementData MovementData;

	UPROPERTY(Transient)
	TObjectPtr<ULrAnimationComponent> AnimationComponent;
};
