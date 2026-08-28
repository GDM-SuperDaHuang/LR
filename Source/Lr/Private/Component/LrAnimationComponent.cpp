// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LrAnimationComponent.h"

#include "Mover/LrMoverComponent.h"
#include "Pawn/LrPawnBase.h"

// Sets default values for this component's properties
ULrAnimationComponent::ULrAnimationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	// SetComponentTickInterval(0.3f);
	// ...
}

void ULrAnimationComponent::BeginPlay()
{
	Super::BeginPlay();
	CachedPawn = Cast<ALrPawnBase>(GetOwner());
	CachedPawn = GetOwner<ALrPawnBase>();
	if (CachedPawn)
	{
		CachedMover = CachedPawn->FindComponentByClass<ULrMoverComponent>();
	}
}
