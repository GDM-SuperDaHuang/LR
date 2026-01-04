// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrPawnBase.h"

// Sets default values
ALrPawnBase::ALrPawnBase()
{
	MoverComponent = CreateDefaultSubobject<UMoverComponent>(TEXT("MoverComponent"));
	MoverComponent->SetupAttachment(GetRootComponent());

    CachedMoveInput = FVector2D::ZeroVector;
    bJumpInput = false;
    bDashInput = false;
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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


/** 移动相关  */
void ALrPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ALrPawnBase::UpdateMove(const FVector2D& Input)
{
	CachedMoveInput = *Input
}

void ALrPawnBase::SetJump(const bool Input)
{
	bJumpInput = *Input
}

void ALrPawnBase::SetDash(const bool Input)
{
	bDashInput = *Input
}
/** 移动相关  */