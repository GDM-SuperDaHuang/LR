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

	//测试
	void TestBreak() const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGeometryCollectionComponent> GeometryCollection;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULrDestructionComponent> Destruction;
};
