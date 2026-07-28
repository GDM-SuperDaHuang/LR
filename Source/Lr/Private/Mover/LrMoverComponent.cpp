// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/LrMoverComponent.h"

#include "Engine/World.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMovementSettings.h"
#include "Mover/Air/LrAirMovementMode.h"
#include "Mover/Blink/LrBlinkMovementMode.h"
#include "Mover/Blink/LrKnockbackMovementMode.h"
#include "Mover/Climb/LrClimbMovementMode.h"
#include "Mover/Climb/LrInsectClimbMovementMode.h"
#include "Mover/Death/LrDeathMovementMode.h"
#include "Mover/Empty/LrEmptyMovementMode.h"
#include "Mover/Fly/LrFlyMovementMode.h"
#include "Mover/Walk/LrWalkMovementMode.h"

ULrMoverComponent::ULrMoverComponent()
{
	// 注册模式
	MovementModes.Add(LrAllModes::Empty, CreateDefaultSubobject<ULrEmptyMovementMode>(TEXT("LrEmptyMovementMode")));
	MovementModes.Add(LrAllModes::Walk, CreateDefaultSubobject<ULrWalkMovementMode>(TEXT("LrWalkMovementMode")));
	MovementModes.Add(LrAllModes::Air, CreateDefaultSubobject<ULrAirMovementMode>(TEXT("LrAirMovementMode")));
	MovementModes.Add(LrAllModes::Blink, CreateDefaultSubobject<ULrBlinkMovementMode>(TEXT("LrBlinkMovementMode")));
	MovementModes.Add(LrAllModes::Knock, CreateDefaultSubobject<ULrKnockbackMovementMode>(TEXT("LrKnockbackMovementMode")));
	MovementModes.Add(LrAllModes::Death, CreateDefaultSubobject<ULrDeathMovementMode>(TEXT("LrDeathMovementMode")));
	MovementModes.Add(LrAllModes::Fly, CreateDefaultSubobject<ULrFlyMovementMode>(TEXT("LrFlyMovementMode")));
	MovementModes.Add(LrAllModes::Climb, CreateDefaultSubobject<ULrClimbMovementMode>(TEXT("LrClimbMovementMode")));
	MovementModes.Add(LrAllModes::ClimbInsect, CreateDefaultSubobject<ULrInsectClimbMovementMode>(TEXT("LrInsectClimbMovementMode")));


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

bool ULrMoverComponent::TryEnterClimb() const
{
	const FVector Start = GetOwner()->GetActorLocation();
	const FVector Forward = GetOwner()->GetActorForwardVector();
	const FVector End = Start + Forward * 80.f;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	FHitResult OutHit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_WorldStatic,
		Params
	);

	if (!bHit)
	{
		return false;
	}


	// 判断是否可攀爬
	const float Dot = FVector::DotProduct(OutHit.Normal, FVector::UpVector);
	if (Dot > 0.75f)
	{
		return false;
	}
	// 保存墙信息
	// ClimbWallNormal = OutHit.Normal;

	return true;
}
