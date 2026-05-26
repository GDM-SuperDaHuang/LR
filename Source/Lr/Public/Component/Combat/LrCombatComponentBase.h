// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LrCombatComponentBase.generated.h"


UENUM(BlueprintType)
enum class ELrCombatShape : uint8
{
	Sphere,
	Box,
	Cone
};

USTRUCT(BlueprintType)
struct FLrCombatQueryParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELrCombatShape ShapeType = ELrCombatShape::Sphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Origin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Forward = FVector::ForwardVector;

	// Sphere
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 300.f;

	// Box
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BoxExtent = FVector(100.f);

	// Cone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConeHalfAngle = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConeLength = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIgnoreSelf = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIgnoreTeammates = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrawDebug = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrCombatComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UPROPERTY()
	TWeakObjectPtr<AActor> CachedTargetActor; //目标可能死亡

	// Sets default values for this component's properties
	ULrCombatComponentBase();
	// Sets default values for this component's properties
	TArray<AActor*> PerformCombatQuery(const FLrCombatQueryParams& Params);
	TArray<AActor*> PerformSphereQuery(const FLrCombatQueryParams& Params);
	TArray<AActor*> PerformBoxQuery(const FLrCombatQueryParams& Params);
	TArray<AActor*> PerformConeQuery(const FLrCombatQueryParams& Params);
	// 目标过滤
	bool IsValidCombatTarget(AActor* TargetActor, const FLrCombatQueryParams& Params) const;
	// 角度判断
	bool IsInsideCone(const FVector& Origin, const FVector& Forward, const FVector& TargetLocation, float ConeHalfAngleDeg) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
