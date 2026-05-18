// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LrAICombatState.generated.h"

UENUM(BlueprintType)
enum class ELrAICombatState : uint8
{
	Idle,

	Chase,

	Attack,

	Dead,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrAICombatState : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULrAICombatState();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	
	UPROPERTY(VisibleAnywhere)
	ELrAICombatState CombatState;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	void SetCombatState(ELrAICombatState NewState);

	ELrAICombatState GetCombatState() const;
};
