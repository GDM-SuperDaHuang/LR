// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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


namespace LrInputID
{
	constexpr int32 Jump = 1;

	constexpr int32 Hold = 100;
	
}
