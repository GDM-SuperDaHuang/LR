// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/LrPlayerPawn.h"
#include "MoverComponent.h"

ALrPlayerPawn::ALrPlayerPawn()
{
	MoverComponent = CreateDefaultSubobject<UMoverComponent>(TEXT("MoverComponent"));
	// todo 初始化状态
	MoverComponent->SetupAttachment(GetRootComponent());

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
}

void ALrPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


// Called every frame
void ALrPlayerPawn::ProduceMoverInput(float DeltaTime)
{
	FMoverInputCmd* Cmd = static_cast<FMoverInputCmd*>(InputContext);

    // 填充到 OnGenerateMove 的 FMoverSimulationTickParams Params 的 Params.InputCmdContext
    // OnSimulationTick 的 FMoverSimulationTickParams不保证 InputCmdContext 有意义，不推荐使用
    Cmd->MoveInput = CachedMoveInput;
    Cmd->bJumpPressed = bJumpPressed;
    Cmd->bDashPressed = bDashPressed;

    // 填充完毕，重置状态
    bJumpPressed = false;
    bDashPressed = false;
}
