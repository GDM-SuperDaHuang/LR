// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LrAnimInstance.h"

#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#include "Component/LrAnimationComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Pawn.h"
#include "Pawn/LrPawnBase.h"

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
	if (MovementData.Status == 1)
	{
		Montage_Stop(0.1f);
	}
	if (MovementData.bIsFly) //播放声音
	{
		// 如果已经在播放，则不重复播放
		if (AnimationComponent->CachedPawn->FlyLoopSoundComponent->IsPlaying())
		{
			return;
		}
		// 设置声音并播放（循环）
		AnimationComponent->CachedPawn->FlyLoopSoundComponent->SetSound(AnimationComponent->CachedPawn->FlyLoopSound);
		AnimationComponent->CachedPawn->FlyLoopSoundComponent->bAutoActivate = true;
		AnimationComponent->CachedPawn->FlyLoopSoundComponent->Play();
	}
	else //关闭声音
	{
		// 如果已经在播放，则不重复播放
		if (AnimationComponent->CachedPawn->FlyLoopSoundComponent->IsPlaying())
		{
			AnimationComponent->CachedPawn->FlyLoopSoundComponent->Stop();
		}
	}
}
