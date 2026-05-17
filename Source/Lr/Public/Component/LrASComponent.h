// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LrASComponent.generated.h"
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnASChanged,
	float,
	float
);
// ASC->GetGameplayAttributeValueChangeDelegate() 可以代替ULrASComponent
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LR_API ULrASComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	// ULrASComponent();

public:
	float GetHealth() const { return HP; }
	float GetMaxHealth() const { return MaxHP; }

	float GetMana() const { return MP; }
	float GetMaxMana() const { return MaxMP; }

public:

	void ApplyDamage(float Damage);

	void ConsumeMana(float Cost);
public:
	FOnASChanged OnHealthChanged;
	FOnASChanged OnManaChanged;

// protected:
// 	// Called when the game starts
// 	virtual void BeginPlay() override;
//
// public:	
// 	// Called every frame
// 	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:

	float HP = 100.f;
	float MaxHP = 100.f;

	float MP = 100.f;
	float MaxMP = 100.f;
};
