// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mover/FLrMoverInputCmd.h"
#define ECC_Projectile ECollisionChannel::ECC_GameTraceChannel1

// AI 
namespace LrBBKeys
{
	/** 当前检测到的目标 Actor */
	static const FName TargetActor = TEXT("TargetActor");

	/** 随机巡逻/追击目标位置 */
	static const FName MoveLocation = TEXT("MoveLocation");

	const FName LastKnownLocation = TEXT("LastKnownLocation");

	const FName AIState = TEXT("AIState");

	const FName HomeLocation = TEXT("HomeLocation");
};

namespace LrGEKeys
{
	static const FName Flags = TEXT("1");
	static const FName SpeedCutRate = TEXT("2");
	static const FName Duration = TEXT("3");
	static const FName DamageValue = TEXT("4");
};


UENUM(BlueprintType)
enum class EGAInputID : uint8
{
	None = 0 UMETA(Hidden), // 通常隐藏"空"选项
	Jump = 1,
	NormalMelee = 2,
	Burt = 3,
	Blink = 4,

	Switch = 99, //切换武器操作
	Lightning = 100,
};

namespace LrInputID
{
	constexpr int32 Hold = static_cast<int32>(EGAInputID::Lightning) - 1;
}
