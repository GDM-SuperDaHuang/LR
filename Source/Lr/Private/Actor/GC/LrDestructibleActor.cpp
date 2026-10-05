// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/GC/LrDestructibleActor.h"

#include "Component/GC/LrDestructionComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

// Sets default values
ALrDestructibleActor::ALrDestructibleActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	RootComponent = GeometryCollection;
	Destruction = CreateDefaultSubobject<ULrDestructionComponent>(TEXT("Destruction"));
	Destruction->SetGeometryCollection(GeometryCollection);
	//防止一开始就碎裂
	GeometryCollection->SetSimulatePhysics(true);
	GeometryCollection->ObjectType = EObjectStateTypeEnum::Chaos_Object_Static;
	GeometryCollection->SetEnableGravity(false);
	GeometryCollection->SetCollisionProfileName(TEXT("BlockAll"));
}

void ALrDestructibleActor::TestBreak() const
{
	if (!Destruction)
	{
		return;
	}
	GeometryCollection->SetDynamicState(Chaos::EObjectStateType::Dynamic);
	// GeometryCollection->SetSimulatePhysics(true);
	GeometryCollection->SetEnableGravity(true);

	const FVector HitPoint = GetActorLocation();

	const FVector Direction = FVector::ForwardVector;

	Destruction->ApplyImpactSimple(
		HitPoint,
		Direction,
		1500.f,
		100.f);
}
