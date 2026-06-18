// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/LrTickableWorldSubsystem.h"

#include "Actor/Projectile/LrProjectile.h"
#include "UI/Widget/Bar/WorldBar/LrWorldBarWidget.h"

TStatId ULrTickableWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULrWorldBarSubsystem, STATGROUP_Tickables);
}

/**
 * 集中刷新
 * @param DeltaTime 
 */
void ULrTickableWorldSubsystem::Tick(float DeltaTime)
{
	for (int32 i = ActiveBars.Num() - 1; i >= 0; --i)
	{
		ULrWorldBarWidget* Bar = ActiveBars[i].Get();
		if (!Bar)
		{
			ActiveBars.RemoveAtSwap(i);
			continue;
		}
		Bar->Tick(DeltaTime);
		if (FMath::IsNearlyEqual(Bar->LastGhostPercent, Bar->TargetPercent, 0.001f))
		{
			ActiveBars.RemoveAtSwap(i);
			Bar->bIsAnimating = false;
		}
	}
}

void ULrTickableWorldSubsystem::RegisterActiveBar(ULrWorldBarWidget* Bar)
{
	ActiveBars.AddUnique(Bar);
}

ALrProjectile* ULrTickableWorldSubsystem::AcquireProjectile()
{
	if (FreeProjectiles.Num() > 0)
	{
		ALrProjectile* Projectile = FreeProjectiles.Pop(false);
		return Projectile;
	}

	if (!ProjectileClass)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ALrProjectile* NewProjectile = GetWorld()->SpawnActor<ALrProjectile>(
		ProjectileClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		Params);

	if (!NewProjectile)
	{
		return nullptr;
	}

	AllProjectiles.Add(NewProjectile);
	return NewProjectile;
}

void ULrTickableWorldSubsystem::ReleaseProjectile(ALrProjectile* Projectile)
{
	if (!Projectile)
	{
		return;
	}
	FreeProjectiles.Add(Projectile);
}

void ULrTickableWorldSubsystem::SpawnProjectile(AActor* InOwner, const FVector& StartLocation, const FVector& Direction)
{
	if (!GetWorld())
	{
		return;
	}

	//客户端不生成
	if (GetWorld()->GetNetMode() == NM_Client)
	{
		return;
	}

	ALrProjectile* Projectile = AcquireProjectile();

	if (!Projectile)
	{
		return;
	}

	Projectile->ActivateProjectile(
		StartLocation,
		Direction,
		InOwner);
}
