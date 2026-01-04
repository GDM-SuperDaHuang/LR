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
	LrInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,  &ALrPlayerController::Move);
	LrInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ALrPlayerController::JumpPress);
	LrInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &ALrPlayerController::DashPress);

	// 技能释放相关
	AuraInputComponent->BindAbilityActions(
		InputConfig, // 数据资产里配了 IA <-> Tag 表
		this,
		&ThisClass::AbilityInputTagPressed,
		&ThisClass::AbilityInputTagReleased, 
		ThisClass::AbilityInputTagHeld);
}

void ALrPlayerController::Move(const FInputActionValue& InputActionValue) 
{
	if (LrPawnBase* LrPawn = GetPawn<LrPawnBase>()){
		LrPawn.UpdateMove(InputActionValue.Get<FVector2D>())
	}
}

void ALrPlayerController::JumpPress(const FInputActionValue& InputActionValue) 
{
	if (LrPawnBase* LrPawn = GetPawn<LrPawnBase>()){
		LrPawn.SetJump(true)
	}
}

void ALrPlayerController::DashPress(const FInputActionValue& InputActionValue) 
{
	if (LrPawnBase* LrPawn = GetPawn<LrPawnBase>()){
		LrPawn.SetDash(true)
	}
}

void ALrPlayerController::AbilityInputTagPressed(FGameplayTag InputTag) 
{

}

void ALrPlayerController::AbilityInputTagReleased(FGameplayTag InputTag) 
{

}

void ALrPlayerController::AbilityInputTagHeld(FGameplayTag InputTag) 
{

}