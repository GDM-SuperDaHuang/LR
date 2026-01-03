// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrPawnBase.h"

// Sets default values
ALrPawnBase::ALrPawnBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALrPawnBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALrPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ALrPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

