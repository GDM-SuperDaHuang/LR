// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LrPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "ASC/LrASC.h"
#include "Lib/LrCommonLibrary.h"
#include "Pawn/LrHeroPawn.h"
#include "Player/Input/LrInputComponent.h"
#include "Tags/LrGameplayTags.h"

void ALrPlayerController::BeginPlay()
{
	Super::BeginPlay();
	//检查是否有绑定，失败则崩溃
	check(AuraContext);

	// 拿到本地玩家的 EnhancedInput 子系统，用于动态挂/卸映射上下文
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	// check(Subsystem);单人游戏
	// 将输入映射上下文（AuraContext）添加到子系统，优先级为0（数值越小优先级越高）
	if (Subsystem) //多人游戏
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}


	bShowMouseCursor = true; //显示鼠标光标
	DefaultMouseCursor = EMouseCursor::Default; // 设置默认鼠标光标样式


	// 输入模式：既响应游戏（WASD）也响应 UI（点击 Widget）
	// FInputModeGameAndUI InputModeData;
	// InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 不锁定鼠标到视口
	// InputModeData.SetHideCursorDuringCapture(false); // 捕获输入时不隐藏光标
	// SetInputMode(InputModeData); // 应用输入模式
}

void ALrPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 销毁默认的
	if (InputComponent)
	{
		InputComponent->DestroyComponent();
	}
	// 创建你自己的 InputComponent
	InputComponent = NewObject<ULrInputComponent>(this, ULrInputComponent::StaticClass());
	InputComponent->RegisterComponent();

	// 我们自定义的 UAuraInputComponent 在 BP 里已经挂载，直接强转
	ULrInputComponent* AuraInputComponent = CastChecked<ULrInputComponent>(InputComponent);
	check(AuraInputComponent);

	/**
	 * 普通移动轴绑定 
	 * ETriggerEvent::Started:(按下：调用,只这一下)
	 * ETriggerEvent::Triggered:(每帧调用,按下:调用, 按住:调用)
	 * ETriggerEvent::Completed:(松开：调用)
	 * ETriggerEvent::Ongoing:(特定条件满足时：每帧调用（如长按未达标）)
	 * ETriggerEvent::Canceled:(条件中断：调用一次（异常取消）)
	 */
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALrPlayerController::Move);

	// 停止
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ALrPlayerController::MoveCompleted);

	// 一键批量绑定所有“技能输入 Tag”到三个回调
	AuraInputComponent->BindAbilityActions(
		InputConfig, // 数据资产里配了 IA <-> Tag 表
		this,
		&ThisClass::AbilityInputTagPressed,
		&ThisClass::AbilityInputTagReleased,
		&ThisClass::AbilityInputTagHeld);
}

// 按下
void ALrPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (LrASC == nullptr)
	{
		LrASC = Cast<ULrASC>(ULrCommonLibrary::GetASC(GetPawn()));
		if (LrASC == nullptr) return;
	}
	LrASC->AbilityInputTagPressed(InputTag);
}

// 释放
void ALrPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (LrASC == nullptr)
	{
		LrASC = Cast<ULrASC>(ULrCommonLibrary::GetASC(GetPawn()));
		if (LrASC == nullptr) return;
	}
	LrASC->AbilityInputTagReleased(InputTag);
}

// 一直按
void ALrPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (LrASC == nullptr)
	{
		LrASC = Cast<ULrASC>(ULrCommonLibrary::GetASC(GetPawn()));
		if (LrASC == nullptr) return;
	}
	LrASC->AbilityInputTagHeld(InputTag);
}

void ALrPlayerController::Move(const FInputActionValue& InputActionValue)
{
	ULrCommonLibrary::PrintLog(GetWorld(), this);
	if (LrASC == nullptr)
	{
		TObjectPtr<APawn> tPawn = GetPawn();
		LrASC = Cast<ULrASC>(ULrCommonLibrary::GetASC(GetPawn()));
		if (LrASC == nullptr) return;
	}
	FVector2D Input;
	bool bIsBlockMove = LrASC->HasMatchingGameplayTag(FLrGameplayTags::Get().State_Block_Move);
	if (bIsBlockMove)
	{
		Input = FVector2D::ZeroVector;
	}
	else
	{
		// 增强输入默认返回 FVector2D
		Input = InputActionValue.Get<FVector2D>();
	}

	if (ALrPawnBase* ControlledPawn = GetPawn<ALrPawnBase>())
	{
		ControlledPawn->UpdateMove(Input);
	}
	// //返回 摄像机或控制器 的 世界旋转
	// const FRotator Rotator = GetControlRotation();
	// // 提取 Yaw 旋转，忽略 Pitch/Roll
	// const FRotator YawRotator(0.f, Rotator.Yaw, 0.f); //水平面朝向（俯仰角清零），防止 上坡/下坡 时 前后方向错位。
	//
	// // 把二维输入映射到世界空间的前/右方向
	// const FVector ForwardDirection = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X);
	// const FVector RightDirection = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y);
}

void ALrPlayerController::MoveCompleted(const FInputActionValue& InputActionValue)
{
	// 增强输入默认返回 FVector2D
	const FVector2D Input = FVector2D::ZeroVector;
	if (ALrPawnBase* ControlledPawn = GetPawn<ALrPawnBase>())
	{
		ControlledPawn->UpdateMove(Input);
	}
	// //返回 摄像机或控制器 的 世界旋转
	// const FRotator Rotator = GetControlRotation();
	// // 提取 Yaw 旋转，忽略 Pitch/Roll
	// const FRotator YawRotator(0.f, Rotator.Yaw, 0.f); //水平面朝向（俯仰角清零），防止 上坡/下坡 时 前后方向错位。
	//
	// // 把二维输入映射到世界空间的前/右方向
	// const FVector ForwardDirection = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X);
	// const FVector RightDirection = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y);
}

void ALrPlayerController::Jump(const FInputActionValue& InputActionValue)
{
	if (ALrPawnBase* ControlledPawn = GetPawn<ALrPawnBase>())
	{
		ControlledPawn->UpdatePressedJump(true);
	}
}
