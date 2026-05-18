// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace LrBBKeys
{
	/** 当前检测到的目标 Actor */
	static const FName TargetActor = TEXT("TargetActor");

	/** 随机巡逻/追击目标位置 */
	static const FName MoveLocation = TEXT("MoveLocation");

	/** 当前是否处于可攻击的战斗状态 */
	static const FName CombatState = TEXT("CombatState");
};