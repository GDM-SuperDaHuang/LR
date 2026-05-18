// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LrAnimationComponent.generated.h"

class ALrPawnBase;
class ULrMoverComponent;

USTRUCT(BlueprintType)
struct FLrAnimMovementData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector Velocity = FVector::ZeroVector;

	//每帧调用
	UPROPERTY(BlueprintReadOnly)
	float Speed = 0.f; //速度大小

	UPROPERTY(BlueprintReadOnly)
	float Direction = 0.f; //方向


	//事件驱动
	UPROPERTY(BlueprintReadOnly)
	bool bIsJumping = false; //是否跳跃

	UPROPERTY(BlueprintReadOnly)
	bool bIsBlink = false; //是否冲刺

	UPROPERTY(BlueprintReadOnly)
	bool bIsCrouching = false; //是否蹲下

	UPROPERTY(BlueprintReadOnly)
	bool bIsFalling = false; //是否掉落
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULrAnimationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	FLrAnimMovementData& GetMovementData()
	{
		return MovementData;
	}

protected:
	void UpdateMovementData(float DeltaTime);

protected:
	UPROPERTY()
	TObjectPtr<ALrPawnBase> CachedPawn;

	UPROPERTY()
	TObjectPtr<ULrMoverComponent> CachedMover;

	UPROPERTY(BlueprintReadOnly, Category="Animation")
	FLrAnimMovementData MovementData;
};
