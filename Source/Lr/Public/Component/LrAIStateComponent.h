// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LrAIStateComponent.generated.h"

UENUM(BlueprintType)
enum class ELrAIState : uint8
{
	Idle, //待机
	Patrol, //巡逻
	Search, //丢失目标搜索
	Chase, //追击
	Combat, //战斗（进入攻击范围）
	Dead, //死亡
	ReturnHome, //回家
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAIStateChanged, ELrAIState, ELrAIState);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrAIStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULrAIStateComponent();

public:
	void SetAIState(ELrAIState NewState);
	UFUNCTION(BlueprintPure)
	ELrAIState GetAIState() const { return CurrentState; }

	bool IsCombatState() const;

	FOnAIStateChanged OnAIStateChanged;

private:
	UPROPERTY(VisibleAnywhere)
	ELrAIState CurrentState = ELrAIState::Idle;
};
