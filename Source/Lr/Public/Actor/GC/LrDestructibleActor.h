// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LrDestructibleActor.generated.h"

class ULrDestructionComponent;

UCLASS()
class LR_API ALrDestructibleActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALrDestructibleActor();

	virtual void BeginPlay() override;
	//测试
	// void TestBreak() const;
	// UPROPERTY(EditAnywhere)
	// bool bTestBreak =false;

	// 子弹 / 投射物命中统一入口：唤醒 + 碎裂
	UFUNCTION(BlueprintCallable, Category = "LR|Destruction")
	bool NotifyBulletHit(const FHitResult& Hit, const FVector& BulletVelocity, float Damage = 100.f);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGeometryCollectionComponent> GeometryCollection;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULrDestructionComponent> Destruction;
};
