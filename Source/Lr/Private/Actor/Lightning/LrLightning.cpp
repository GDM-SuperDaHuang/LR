// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Lightning/LrLightning.h"

// Sets default values
ALrLightning::ALrLightning()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALrLightning::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALrLightning::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

