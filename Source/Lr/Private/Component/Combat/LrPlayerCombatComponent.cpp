// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Combat/LrPlayerCombatComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Pawn/LrHeroPawn.h"
#include "Player/LrPlayerController.h"

// Sets default values for this component's properties
ULrPlayerCombatComponent::ULrPlayerCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TWeakObjectPtr<ALrPawnBase> ULrPlayerCombatComponent::GetNearestPawnToCursor()
{
	TCopyQualifiersFromTo_T<UObject, ALrHeroPawn>* LrHeroPawn = Cast<ALrHeroPawn>(GetOuter());
	ALrPlayerController* PC = Cast<ALrPlayerController>(LrHeroPawn->GetController());
	ALrPawnBase* NearestPawnToCursor = PC->GetNearestPawnToCursor();
	return NearestPawnToCursor;
}

void ULrPlayerCombatComponent::GetAimPoint(FLrAimData& OutAimData)
{
	ALrHeroPawn* Hero = Cast<ALrHeroPawn>(GetOuter());
	if (!Hero)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Hero->GetController());

	if (!PC)
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
	OutAimData.ViewLocation = ViewLocation;
	OutAimData.ViewDirection = ViewRotation.Vector();

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + OutAimData.ViewDirection * 15000.f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Hero);
	Params.bTraceComplex = true;

	FHitResult Hit;
	OutAimData.bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		Params);

	if (OutAimData.bHit)
	{
		OutAimData.HitResult = Hit;
		OutAimData.AimPoint = Hit.ImpactPoint;
	}
	else
	{
		OutAimData.AimPoint = TraceEnd;
	}

	DrawDebugCylinder(
		GetWorld(),
		OutAimData.ViewLocation,
		OutAimData.AimPoint,
		10.f,
		12,
		FColor::Red,
		false,
		2.f);

	// #if ENABLE_DRAW_DEBUG
	//
	// 	DrawDebugLine(
	// 		GetWorld(),
	// 		TraceStart,
	// 		TraceEnd,
	// 		FColor::Red,
	// 		false,
	// 		0.05f,
	// 		0,
	// 		1.f);
	//
	// 	DrawDebugSphere(
	// 		GetWorld(),
	// 		OutAimData.AimPoint,
	// 		10.f,
	// 		16,
	// 		FColor::Green,
	// 		false,
	// 		0.05f);
	//
	// #endif
}
