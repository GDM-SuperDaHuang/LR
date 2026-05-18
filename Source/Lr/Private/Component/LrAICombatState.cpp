// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LrAICombatState.h"

// Sets default values for this component's properties
ULrAICombatState::ULrAICombatState()
{
	PrimaryComponentTick.bCanEverTick = false;
	CombatState = ELrAICombatState::Idle;
}


// Called when the game starts
void ULrAICombatState::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void ULrAICombatState::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void ULrAICombatState::SetCombatState(ELrAICombatState NewState)
{
	CombatState = NewState;
}

ELrAICombatState ULrAICombatState::GetCombatState() const
{
	return CombatState;
}

