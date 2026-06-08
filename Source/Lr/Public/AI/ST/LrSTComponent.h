// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeComponent.h"
#include "LrSTComponent.generated.h"
USTRUCT(BlueprintType)
struct FEnemyAIExternalData
{
	GENERATED_BODY()
	// AI 自己
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy AI")
	APawn* SelfPawn = nullptr;

	// 当前目标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy AI")
	AActor* TargetActor = nullptr;

	// 当前血量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy AI")
	float Health = 100.f;

	// 攻击范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy AI")
	float AttackRange = 200.f;

	// 巡逻点
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy AI")
	TArray<FVector> PatrolPoints;

	// 当前巡逻点索引
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy AI")
	int32 CurrentPatrolIndex = 0;
};

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrSTComponent : public UStateTreeComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FEnemyAIExternalData EnemyData;
	
	ULrSTComponent()
	{
		PrimaryComponentTick.bCanEverTick = true;
	}

	virtual void BeginPlay() override
	{
		Super::BeginPlay();

		if (StateTreeRef.GetStateTree())
		{
			StartLogic();
		}
	}
};
