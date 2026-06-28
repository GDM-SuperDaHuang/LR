// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASC/GE/LrGEContext.h"
#include "GameFramework/Actor.h"
#include "LrProjectile.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
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
	//根组件
	UPROPERTY()
	TObjectPtr<USceneComponent> HomingTargetSceneComponent;

	// 选择球形
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	// 发射物模型
	UPROPERTY(EditAnywhere, Category="Info")
	TObjectPtr<UStaticMeshComponent> LrMesh;
	UPROPERTY(EditAnywhere, Category="Info")
	TObjectPtr<UNiagaraComponent> LrNiagaraComponent; //同一个 Component 只能播放一个 NiagaraSystem。
	 
	// 命中特效
	UPROPERTY(EditAnywhere, Category="Info")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	// 命中声音
	UPROPERTY(EditAnywhere, Category="Info")
	TObjectPtr<USoundBase> ImpactSound;
	

protected:
	virtual void BeginPlay() override;

protected:
	// UFUNCTION()
	// void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	void DeactivateProjectile();

	UPROPERTY()
	TObjectPtr<AActor> ProjectileOwner;
	UPROPERTY()
	FDamageEffectParams DamageEffectParams;


public:
	UPROPERTY(EditDefaultsOnly)
	float Speed = 600.f;
	
};
