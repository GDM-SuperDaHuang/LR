// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

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
