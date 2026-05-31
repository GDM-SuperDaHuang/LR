// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LrAnimInstance.h"

#include "Component/LrAnimationComponent.h"
#include "Mover/LrMoverComponent.h"

void ULrAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
	{
		return;
	}
	AnimationComponent = Pawn->FindComponentByClass<ULrAnimationComponent>();

	// 订阅数据变化委托（事件驱动，避免每帧轮询）
	if (AnimationComponent)
	{
		AnimationComponent->OnMovementDataChanged.BindUObject(this, &ULrAnimInstance::OnMovementDataUpdated);
	}
}

void ULrAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	// 事件驱动：数据通过 OnMovementDataUpdated 回调更新
	// 这里不再每帧调用 GetMovementData()，减少函数调用开销
}

void ULrAnimInstance::OnMovementDataUpdated(const FLrAnimMovementData& NewData)
{
	MovementData = NewData;
}
