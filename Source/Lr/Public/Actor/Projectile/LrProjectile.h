// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/LrExcelConfig.h"
#include "GameFramework/Actor.h"
#include "LrProjectile.generated.h"

struct FLrProjectileConfigRow;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class LR_API ALrProjectile : public AActor
{
	GENERATED_BODY()

public:
	ALrProjectile();

protected:
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// 选择球形
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> Movement;

	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundComponent;
protected:
	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
	void ActivateProjectile(const FVector& StartLocation, const FVector& Direction, AActor* InOwner);

	void DeactivateProjectile();

public:
	UPROPERTY(ReplicatedUsing=OnRep_Active)
	bool bActive = false;
	UFUNCTION()
	void OnRep_Active();

	UPROPERTY()
	TObjectPtr<AActor> ProjectileOwner;

public:
	UPROPERTY(EditDefaultsOnly)
	float Damage = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float Speed = 2500.f;

	UPROPERTY(EditDefaultsOnly)
	float LifeTime = 5.f;
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> LoopingSound;
private:
	FTimerHandle LifeTimer;
};
