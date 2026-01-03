// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LrPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Input/LrInputComponent.h"

void ALrPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(LrInputMappingContext);
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	// 将输入映射上下文（AuraContext）添加到子系统，优先级为0（数值越小优先级越高）
	if (Subsystem) //多人游戏
	{
		Subsystem->AddMappingContext(LrInputMappingContext, 0);
	}

	// todo 

}

void ALrPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	ULrInputComponent* LrInputComponent = CastChecked<ULrInputComponent>(InputComponent);
	check(LrInputComponent);

	// 普通移动轴绑定
	LrInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,  &ALrPlayerController::Move );
	// // Shift 按下/松开（用来区分“强制施法”与“点地移动”）
	// LrInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &ALrPlayerController::ShiftPressed);
	// LrInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &ALrPlayerController::ShiftReleased);

	

}

void ALrPlayerController::Move(const FInputActionValue& InputActionValue) 
{
	// 增强输入默认返回 FVector2D
	const FVector2D InputAxisVecter = InputActionValue.Get<FVector2D>();

	//返回 摄像机或控制器 的 世界旋转
	const FRotator Rotator = GetControlRotation();
	// 提取 Yaw 旋转，忽略 Pitch/Roll
	const FRotator YawRotator(0.f, Rotator.Yaw, 0.f); //水平面朝向（俯仰角清零），防止 上坡/下坡 时 前后方向错位。

	// 把二维输入映射到世界空间的前/右方向
	const FVector ForwardDirection = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y);
	if (APawn* PawnController = GetPawn<APawn>())
	{
		PawnController->AddMovementInput(ForwardDirection, InputAxisVecter.X);
		PawnController->AddMovementInput(RightDirection, InputAxisVecter.Y);
	}
}
