// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/LrPlayerPawn.h"
#include "MoverComponent.h"

ALrPlayerPawn::ALrPlayerPawn()
{
	MoverComponent = CreateDefaultSubobject<UMoverComponent>(TEXT("MoverComponent"));
	// todo 初始化状态

	// const FMyMoverInput* Input = Params.InputCmdContext ? static_cast<const FMyMoverInput*>(Params.InputCmdContext) : nullptr;
	// FVector Direction = FVector::ZeroVector;
	// if (Input)
	// {
	// 	Direction = FVector(Input->MoveDirection.X, Input->MoveDirection.Y, 0.f).GetClampedToMaxSize(1.f);
	// }
	// OutMove.LinearVelocity = Direction * 600.f;
}

void ALrPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// RootComponent = MoverComponent;// 设置为根组件（处理物理/位置）
}

void ALrPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
