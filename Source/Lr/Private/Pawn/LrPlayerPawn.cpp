// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/LrPlayerPawn.h"

#include "MoverComponent.h"
#include "Mover/LrBaseMovementMode.h"

ALrPlayerPawn::ALrPlayerPawn()
{
	
}

void ALrPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	if (MoverComponent)
	{
		// 注册到Network Prediction驱动自动同步和预测
		MoverComponent->RegisterNetworkPrediction();

		// 注册运动模式
		MoverComponent->AddMovementModeFromClass(TEXT("Base"), UBaseMovementMode::StaticClass());
		MoverComponent->AddMovementModeFromClass(TEXT("Jump"), UJumpMovementMode::StaticClass());
		MoverComponent->AddMovementModeFromClass(TEXT("Dash"), UDashMovementMode::StaticClass());

		MoverComponent->QueueNextMode(TEXT("Base"));

		// 绑定输入生产委托
		MoverComponent->BindProduceInputDelegate(
			UMoverComponent::FProduceInputDelegate::CreateUObject(this, &AMyMoverPawn::ProduceMoverInput)
		);
	}
	// RootComponent = MoverComponent;// 设置为根组件（处理物理/位置）
}

void ALrPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ALrPlayerPawn::ProduceMoverInput(float DeltaTimeMS, FMoverInputCmdContext* InputContext)
{
	FLrMoverInputCmd* Cmd = static_cast<FLrMoverInputCmd*>(InputContext);

	// 填充到 OnGenerateMove 的 FMoverSimulationTickParams Params 的 Params.InputCmdContext
	// OnSimulationTick 的 FMoverSimulationTickParams不保证 InputCmdContext 有意义，不推荐使用
	Cmd->MoveInput = CachedMoveInput;
	Cmd->bJumpPressed = bJumpPressed;
	Cmd->bDashPressed = bDashPressed;

	// 填充完毕，重置状态
	bJumpPressed = false;
	bDashPressed = false;
}
