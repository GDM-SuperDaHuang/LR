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
	FVector GetNextPatrolPoint();
protected:

	UPROPERTY(EditInstanceOnly, Category="Patrol")
	TArray<AActor*> PatrolPoints;

	int32 CurrentIndex = 0;
};
