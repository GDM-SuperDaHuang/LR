// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASC/GE/LrGEContext.h"
#include "GameFramework/Actor.h"
#include "LrProjectile.generated.h"

class UAudioComponent;
struct FLrProjectileConfigRow;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class LR_API ALrProjectile : public AActor
{
	GENERATED_BODY()

public:
	ALrProjectile();
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> Movement;
	UPROPERTY()
	TObjectPtr<USceneComponent> HomingTargetSceneComponent;
protected:
	virtual void BeginPlay() override;

// public:
// 	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// 选择球形
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	

	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundComponent;
protected:
	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
	void DeactivateProjectile();

	UPROPERTY()
	TObjectPtr<AActor> ProjectileOwner;
	
	UPROPERTY()
	FDamageEffectParams DamageEffectParams;
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
