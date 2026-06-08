// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LrPatrolRouteComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrPatrolRouteComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULrPatrolRouteComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FVector GetNextPatrolPoint();
	void GenerateCircle(float Radius, int32 Count);
	void DebugDrawPatrolRoute();
	FORCEINLINE void SetHomeLocation(const FVector& InHome)
	{
		HomeLocation = InHome;
	}

	UPROPERTY()
	AActor* TargetActor;
protected:
	int32 CurrentIndex = 0;

	UPROPERTY()
	TArray<FVector> LocalOffsets;

	UPROPERTY()
	FVector HomeLocation = FVector::ZeroVector;

	
};
