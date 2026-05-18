// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/Generator/LrEQSGeneratorCircle.h"

#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"

void ULrEQSGeneratorCircle::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	TArray<FVector> ContextLocations;
	QueryInstance.PrepareContext(UEnvQueryContext_Querier::StaticClass(), ContextLocations);

	if (ContextLocations.IsEmpty())
	{
		return;
	}

	const FVector Center = ContextLocations[0];
	TArray<FNavLocation> ItemCandidates;
	const float StepAngle = 360.f / PointCount;

	for (int32 i = 0; i < PointCount; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * StepAngle);

		FVector Offset;
		Offset.X = FMath::Cos(Angle) * Radius;
		Offset.Y = FMath::Sin(Angle) * Radius;
		Offset.Z = 0.f;

		ItemCandidates.Add(FNavLocation(Center + Offset));
	}

	ProjectAndFilterNavPoints(ItemCandidates, QueryInstance);
	StoreNavPoints(ItemCandidates, QueryInstance);
}