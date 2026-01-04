// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/LrPlayerPawn.h"
#include "MoverComponent.h"

ALrPlayerPawn::ALrPlayerPawn()
{
	
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
