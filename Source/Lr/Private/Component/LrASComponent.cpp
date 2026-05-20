// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LrASComponent.h"


void ULrASComponent::ApplyDamage(float Damage)
{
	HP = FMath::Clamp(HP - Damage,0.f,MaxHP);
	OnHealthChanged.Broadcast(HP,MaxHP);
}

void ULrASComponent::ConsumeMana(float Cost)
{
	MP = FMath::Clamp(MP - Cost,0.f, MaxMP);
	OnManaChanged.Broadcast(MP,MaxMP);
}
