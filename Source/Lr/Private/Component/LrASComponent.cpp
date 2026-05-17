// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LrASComponent.h"

// Sets default values for this component's properties
// ULrASComponent::ULrASComponent()
// {
// 	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
// 	// off to improve performance if you don't need them.
// 	PrimaryComponentTick.bCanEverTick = true;
//
// 	// ...
// }


// Called when the game starts
// void ULrASComponent::BeginPlay()
// {
// 	Super::BeginPlay();
//
// 	// ...
// 	
// }
//
//
// // Called every frame
// void ULrASComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
// {
// 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
// 	// ...
// }

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
