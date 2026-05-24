// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LrPatrolRouteComponent.h"

ULrPatrolRouteComponent::ULrPatrolRouteComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULrPatrolRouteComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	DebugDrawPatrolRoute();
}

FVector ULrPatrolRouteComponent::GetNextPatrolPoint()
{
	if (LocalOffsets.Num() == 0)
	{
		return HomeLocation;
	}

	CurrentIndex++;
	if (CurrentIndex >= LocalOffsets.Num())
	{
		CurrentIndex = 0;
	}

	// 关键：围绕出生点
	return HomeLocation + LocalOffsets[CurrentIndex];
}

void ULrPatrolRouteComponent::GenerateCircle(float Radius, int32 Count)
{
	LocalOffsets.Empty();

	for (int i = 0; i < Count; i++)
	{
		float Angle = (2 * PI * i) / Count;
		LocalOffsets.Add(FVector(
			FMath::Cos(Angle) * Radius,
			FMath::Sin(Angle) * Radius,
			0.f
		));
	}
}


void ULrPatrolRouteComponent::DebugDrawPatrolRoute()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (LocalOffsets.Num() == 0)
	{
		return;
	}

	const float SphereRadius = 30.f;
	const float Duration = 1.f;
	const int32 Segments = 12;

	for (int32 i = 0; i < LocalOffsets.Num(); ++i)
	{
		const FVector WorldPoint = HomeLocation + LocalOffsets[i];

		// 画点
		DrawDebugSphere(
			World,
			WorldPoint,
			SphereRadius,
			Segments,
			FColor::Green,
			false,
			Duration,
			0,
			2.f);

		// 画编号
		DrawDebugString(
			World,
			WorldPoint + FVector(0, 0, 50.f),
			FString::Printf(TEXT("%d"), i),
			nullptr,
			FColor::White,
			Duration,
			false);

		// 连线到下一个点
		const int32 NextIndex = (i + 1) % LocalOffsets.Num();

		const FVector NextPoint = HomeLocation + LocalOffsets[NextIndex];

		DrawDebugLine(
			World,
			WorldPoint,
			NextPoint,
			FColor::Yellow,
			false,
			Duration,
			0,
			2.f);
	}

	// 画Home点
	DrawDebugSphere(
		World,
		HomeLocation,
		100.f,
		16,
		FColor::Red,
		false,
		Duration,
		0,
		3.f);
}

