// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/LrMoverComponent.h"

#include "Mover/LrAllModes.h"
#include "Mover/LrMovementSettings.h"
#include "Mover/Air/LrAirMovementMode.h"
#include "Mover/Blink/LrBlinkMovementMode.h"
#include "Mover/Blink/LrKnockbackMovementMode.h"
#include "Mover/Death/LrDeathMovementMode.h"
#include "Mover/Walk/LrWalkMovementMode.h"


ULrMoverComponent::ULrMoverComponent()
{
	// 注册模式
	MovementModes.Add(LrAllModes::Walk, CreateDefaultSubobject<ULrWalkMovementMode>(TEXT("LrWalkMovementMode")));
	MovementModes.Add(LrAllModes::Air, CreateDefaultSubobject<ULrAirMovementMode>(TEXT("LrAirMovementMode")));
	MovementModes.Add(LrAllModes::Blink, CreateDefaultSubobject<ULrBlinkMovementMode>(TEXT("LrBlinkMovementMode")));
	MovementModes.Add(LrAllModes::Knock, CreateDefaultSubobject<ULrKnockbackMovementMode>(TEXT("LrKnockbackMovementMode")));
	MovementModes.Add(LrAllModes::Death, CreateDefaultSubobject<ULrDeathMovementMode>(TEXT("LrDeathMovementMode")));


	// 开始模式
	StartingMovementMode = LrAllModes::Air;

	// 创建设置，使其在 Blueprint 中可见
	RealisticSettings = CreateDefaultSubobject<ULrMovementSettings>(TEXT("RealisticSettings"));
}

const FPendingLaunchData* ULrMoverComponent::GetPendingLaunchData()
{
	return &PendingLaunchData;
}

void ULrMoverComponent::Launch(FVector Impulse, float Duration)
{
	PendingLaunchData = {Impulse, Duration};
	this->QueueNextMode(LrAllModes::Knock);
}
