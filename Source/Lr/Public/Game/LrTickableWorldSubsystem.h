// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LrTickableWorldSubsystem.generated.h"

class ALrPawnBase;
struct FLrProjectileConfigRow;
class ALrProjectile;
class ULrWorldBarWidget;
/**
 * 
 */
UCLASS()
class LR_API ULrTickableWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual TStatId GetStatId() const override;
	virtual void Tick(float DeltaTime) override;
	//////////////////////////////////////////////////////////////////////////
	// UI
	void RegisterActiveBar(ULrWorldBarWidget* Bar);

private:
	TArray<TWeakObjectPtr<ULrWorldBarWidget>> ActiveBars;
	//////////////////////////////////////////////////////////////////////////
	
	//投射物对象池


public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ALrProjectile> ProjectileClass;
	ALrProjectile* AcquireProjectile();
	void ReleaseProjectile(ALrProjectile* Projectile);
	void SpawnProjectile(AActor* OwnerPawn, const FVector& StartLocation, const FVector& Direction);
private:
	UPROPERTY()
	TArray<TObjectPtr<ALrProjectile>> AllProjectiles;
	UPROPERTY()
	TArray<TObjectPtr<ALrProjectile>> FreeProjectiles;
	
	//投射物对象池
};
