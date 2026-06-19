// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Combat/LrCombatComponentBase.h"

#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "DrawDebugHelpers.h"
#include "Async/ParallelFor.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Interface/LrCombatInterface.h"

// Sets default values for this component's properties
ULrCombatComponentBase::ULrCombatComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}



TWeakObjectPtr<AActor> ULrCombatComponentBase::GetClosestEnemyInCone(const FLrCombatQueryParams& Params)
{
	// 1. 复用现有的扇形 AOE 查询，获取扇形内的所有合法目标
	FLrCombatQueryParams ConeParams = Params;
	ConeParams.ShapeType = ELrCombatShape::Cone; // 强制指定为 Cone 类型

	TArray<TWeakObjectPtr<AActor>> EnemiesInCone = PerformConeQuery(ConeParams);

	if (EnemiesInCone.IsEmpty())
	{
		return nullptr;
	}

	// 2. 遍历结果，通过比较距离的平方（DistSquared）找出最近的目标
	TWeakObjectPtr<AActor> ClosestEnemy = nullptr;
	float MinDistanceSq = BIG_NUMBER; // 初始化为一个极大值

	for (TWeakObjectPtr<AActor> Enemy : EnemiesInCone)
	{
		if (!Enemy.Get()) continue;

		// 计算平方距离，免去开方运算，优化性能
		float CurrentDistanceSq = FVector::DistSquared(Params.Origin, Enemy->GetActorLocation());

		if (CurrentDistanceSq < MinDistanceSq)
		{
			MinDistanceSq = CurrentDistanceSq;
			ClosestEnemy = Enemy;
		}
	}

	// 3. 缓存并返回结果
	return ClosestEnemy;
}

TArray<TWeakObjectPtr<AActor>> ULrCombatComponentBase::PerformCombatQuery(const FLrCombatQueryParams& Params)
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

TArray<TWeakObjectPtr<AActor>> ULrCombatComponentBase::PerformSphereQuery(const FLrCombatQueryParams& Params)
{
	TArray<TWeakObjectPtr<AActor>> ResultActors;

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

	// 并行筛选：当重叠结果较多时使用 ParallelFor 加速
	const int32 OverlapCount = Overlaps.Num();
	if (OverlapCount > 16)
	{
		TArray<uint8> ValidMask;
		ValidMask.SetNumZeroed(OverlapCount);

		ParallelFor(OverlapCount, [&](int32 Index)
		{
			AActor* HitActor = Overlaps[Index].GetActor();
			if (IsValidCombatTarget(HitActor, Params))
			{
				ValidMask[Index] = 1;
			}
		});

		for (int32 i = 0; i < OverlapCount; ++i)
		{
			if (ValidMask[i])
			{
				ResultActors.AddUnique(Overlaps[i].GetActor());
			}
		}
	}
	else
	{
		for (const FOverlapResult& Result : Overlaps)
		{
			AActor* HitActor = Result.GetActor();
			if (IsValidCombatTarget(HitActor, Params))
			{
				ResultActors.AddUnique(HitActor);
			}
		}
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

TArray<TWeakObjectPtr<AActor>> ULrCombatComponentBase::PerformBoxQuery(const FLrCombatQueryParams& Params)
{
	TArray<TWeakObjectPtr<AActor>> ResultActors;
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

	// 并行筛选：当重叠结果较多时使用 ParallelFor 加速
	const int32 OverlapCount = Overlaps.Num();
	if (OverlapCount > 16)
	{
		TArray<uint8> ValidMask;
		ValidMask.SetNumZeroed(OverlapCount);

		ParallelFor(OverlapCount, [&](int32 Index)
		{
			AActor* HitActor = Overlaps[Index].GetActor();
			if (IsValidCombatTarget(HitActor, Params))
			{
				ValidMask[Index] = 1;
			}
		});

		for (int32 i = 0; i < OverlapCount; ++i)
		{
			if (ValidMask[i])
			{
				ResultActors.AddUnique(Overlaps[i].GetActor());
			}
		}
	}
	else
	{
		for (const FOverlapResult& Result : Overlaps)
		{
			AActor* HitActor = Result.GetActor();
			if (IsValidCombatTarget(HitActor, Params))
			{
				ResultActors.AddUnique(HitActor);
			}
		}
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

TArray<TWeakObjectPtr<AActor>> ULrCombatComponentBase::PerformConeQuery(const FLrCombatQueryParams& Params)
{
	TArray<TWeakObjectPtr<AActor>> ResultActors;

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

	// 并行筛选：当重叠结果较多时使用 ParallelFor 加速
	const int32 OverlapCount = Overlaps.Num();
	if (OverlapCount > 16)
	{
		TArray<uint8> ValidMask;
		ValidMask.SetNumZeroed(OverlapCount);

		ParallelFor(OverlapCount, [&](int32 Index)
		{
			AActor* HitActor = Overlaps[Index].GetActor();
			if (!IsValidCombatTarget(HitActor, Params))
			{
				return;
			}

			const FVector TargetLocation = HitActor->GetActorLocation();
			if (IsInsideCone(Params.Origin, Params.Forward, TargetLocation, Params.ConeHalfAngle))
			{
				ValidMask[Index] = 1;
			}
		});

		for (int32 i = 0; i < OverlapCount; ++i)
		{
			if (ValidMask[i])
			{
				ResultActors.AddUnique(Overlaps[i].GetActor());
			}
		}
	}
	else
	{
		for (const FOverlapResult& Result : Overlaps)
		{
			AActor* HitActor = Result.GetActor();
			if (!IsValidCombatTarget(HitActor, Params))
			{
				continue;
			}

			const FVector TargetLocation = HitActor->GetActorLocation();
			if (IsInsideCone(Params.Origin, Params.Forward, TargetLocation, Params.ConeHalfAngle))
			{
				ResultActors.AddUnique(HitActor);
			}
		}
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
