// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Combat/LrCombatComponentBase.h"

#include "Engine/OverlapResult.h"
#include "Interface/LrCombatInterface.h"

// Sets default values for this component's properties
ULrCombatComponentBase::ULrCombatComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;

}

TArray<AActor*> ULrCombatComponentBase::PerformCombatQuery(const FLrCombatQueryParams& Params)
{
	switch (Params.ShapeType)
	{
	case ELrCombatShape::Sphere:
		return PerformSphereQuery(Params);

	case ELrCombatShape::Box:
		return PerformBoxQuery(Params);

	case ELrCombatShape::Cone:
		return PerformConeQuery(Params);
	}

	return {};
}

TArray<AActor*> ULrCombatComponentBase::PerformSphereQuery(const FLrCombatQueryParams& Params)
{
	TArray<AActor*> ResultActors;

	UWorld* World = GetWorld();
	if (!World)
	{
		return ResultActors;
	}

	TArray<FOverlapResult> Overlaps;

	FCollisionShape Shape = FCollisionShape::MakeSphere(Params.Radius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	bool bHit = World->OverlapMultiByObjectType(
		Overlaps,
		Params.Origin,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		Shape,
		QueryParams);

	if (!bHit)
	{
		return ResultActors;
	}

	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* HitActor = Result.GetActor();

		if (!IsValidCombatTarget(HitActor, Params))
		{
			continue;
		}

		ResultActors.AddUnique(HitActor);
	}


	if (Params.bDrawDebug)
	{
		DrawDebugSphere(
			World,
			Params.Origin,
			Params.Radius,
			16,
			FColor::Red,
			false,
			2.f);
	}

	return ResultActors;
}

TArray<AActor*> ULrCombatComponentBase::PerformBoxQuery(const FLrCombatQueryParams& Params)
{
	TArray<AActor*> ResultActors;
	UWorld* World = GetWorld();
	if (!World)
	{
		return ResultActors;
	}

	TArray<FOverlapResult> Overlaps;

	FVector BoxCenter = Params.Origin + Params.Forward * Params.BoxExtent.X;

	FCollisionShape Shape = FCollisionShape::MakeBox(Params.BoxExtent);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	bool bHit = World->OverlapMultiByObjectType(
		Overlaps,
		BoxCenter,
		Params.Forward.ToOrientationQuat(),
		FCollisionObjectQueryParams(ECC_Pawn),
		Shape,
		QueryParams);

	if (!bHit)
	{
		return ResultActors;
	}

	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* HitActor = Result.GetActor();

		if (!IsValidCombatTarget(HitActor, Params))
		{
			continue;
		}

		ResultActors.AddUnique(HitActor);
	}

	if (Params.bDrawDebug)
	{
		DrawDebugBox(
			World,
			BoxCenter,
			Params.BoxExtent,
			Params.Forward.ToOrientationQuat(),
			FColor::Blue,
			false,
			2.f);
	}

	return ResultActors;
}

TArray<AActor*> ULrCombatComponentBase::PerformConeQuery(const FLrCombatQueryParams& Params)
{
	TArray<AActor*> ResultActors;

	UWorld* World = GetWorld();
	if (!World)
	{
		return ResultActors;
	}

	TArray<FOverlapResult> Overlaps;

	FCollisionShape Shape = FCollisionShape::MakeSphere(Params.ConeLength);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	bool bHit = World->OverlapMultiByObjectType(
		Overlaps,
		Params.Origin,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		Shape,
		QueryParams);

	if (!bHit)
	{
		return ResultActors;
	}

	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* HitActor = Result.GetActor();

		if (!IsValidCombatTarget(HitActor, Params))
		{
			continue;
		}

		const FVector TargetLocation =
			HitActor->GetActorLocation();

		if (!IsInsideCone(
			Params.Origin,
			Params.Forward,
			TargetLocation,
			Params.ConeHalfAngle))
		{
			continue;
		}

		ResultActors.AddUnique(HitActor);
	}

	if (Params.bDrawDebug)
	{
		DrawDebugCone(
			World,
			Params.Origin,
			Params.Forward,
			Params.ConeLength,
			FMath::DegreesToRadians(
				Params.ConeHalfAngle),
			FMath::DegreesToRadians(
				Params.ConeHalfAngle),
			16,
			FColor::Green,
			false,
			2.f);
	}

	return ResultActors;
}

bool ULrCombatComponentBase::IsValidCombatTarget(AActor* TargetActor, const FLrCombatQueryParams& Params) const
{
	if (!TargetActor)
	{
		return false;
	}
	
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}
	
	if (Params.bIgnoreSelf && TargetActor == OwnerActor)
	{
		return false;
	}
	
	if (!TargetActor->Implements<ULrCombatInterface>())
	{
		return false;
	}
	
	ILrCombatInterface* TargetCombat = Cast<ILrCombatInterface>(TargetActor);
	ILrCombatInterface* OwnerCombat = Cast<ILrCombatInterface>(OwnerActor);
	if (!TargetCombat || !OwnerCombat)
	{
		return false;
	}
	
	if (TargetCombat->IsDead())
	{
		return false;
	}
	
	if (Params.bIgnoreTeammates)
	{
		if (TargetCombat->GetTeamID() == OwnerCombat->GetTeamID())
		{
			return false;
		}
	}

	return true;
}

bool ULrCombatComponentBase::IsInsideCone(const FVector& Origin, const FVector& Forward, const FVector& TargetLocation, float ConeHalfAngleDeg) const
{
	FVector ToTarget = (TargetLocation - Origin).GetSafeNormal();

	float Dot = FVector::DotProduct(Forward.GetSafeNormal(), ToTarget);

	float MinDot = FMath::Cos(FMath::DegreesToRadians(ConeHalfAngleDeg));

	return Dot >= MinDot;
}


// Called when the game starts
void ULrCombatComponentBase::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

