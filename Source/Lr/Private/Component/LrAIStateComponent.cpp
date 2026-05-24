// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LrAIStateComponent.h"

// Sets default values for this component's properties
ULrAIStateComponent::ULrAIStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentState = ELrAIState::Idle;
}

void ULrAIStateComponent::SetAIState(ELrAIState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	const ELrAIState Prev = CurrentState;

	CurrentState = NewState;

	OnAIStateChanged.Broadcast(Prev, CurrentState);
}

bool ULrAIStateComponent::IsCombatState() const
{
	return CurrentState == ELrAIState::Combat;
}
