// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LrPlayerController.h"

#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "TimerManager.h"
#include "ASC/LrASC.h"
#include "Engine/LocalPlayer.h"
#include "Game/LrGameInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "Lib/LrCommonLibrary.h"
#include "Lr/Lr.h"
#include "Mover/LrMoverComponent.h"
#include "Pawn/LrHeroPawn.h"
#include "Player/Input/LrInputComponent.h"
#include "Tags/LrGameplayTags.h"


ALrPlayerController::ALrPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALrPlayerController::BeginPlay()
{
	Super::BeginPlay();
	//检查是否有绑定，失败则崩溃
	check(LrIMC);

	// 拿到本地玩家的 EnhancedInput 子系统，用于动态挂/卸映射上下文
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	// check(Subsystem);单人游戏
	// 将输入映射上下文（AuraContext）添加到子系统，优先级为0（数值越小优先级越高）
	if (Subsystem) //多人游戏
	{
		Subsystem->AddMappingContext(LrIMC, 0);
	}

	bShowMouseCursor = true; //显示鼠标光标
	DefaultMouseCursor = EMouseCursor::Default; // 设置默认鼠标光标样式

	ULrGameInstance* GI = GetGameInstance<ULrGameInstance>();
	if (GI)
	{
		ULrInputComponent::ApplyPlayerKeyMappings(InputConfig, GI->InputSaveGame, LrIMC);
	}
	FTimerHandle HoverTimer;
	GetWorldTimerManager().SetTimer(
		HoverTimer,
		this,
		&ALrPlayerController::UpdateHoverTarget,
		0.05f,
		true);

	// 输入模式：既响应游戏（WASD）也响应 UI（点击 Widget）
	// FInputModeGameAndUI InputModeData;
	// InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 不锁定鼠标到视口
	// InputModeData.SetHideCursorDuringCapture(false); // 捕获输入时不隐藏光标
	// SetInputMode(InputModeData); // 应用输入模式


	if (ALrHeroPawn* Hero = Cast<ALrHeroPawn>(GetPawn()))
	{
		CurrentCameraYaw = Hero->CameraBoom->GetComponentRotation().Yaw;

		TargetCameraYaw = CurrentCameraYaw;
	}
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
	// 一键批量绑定所有“技能输入 Tag”到三个回调
	// AuraInputComponent->BindAbilityActions(
	// 	InputConfig, // 数据资产里配了 IA <-> Tag 表
	// 	this,
	// 	LrIMC,
	// 	&ThisClass::AbilityInputTagPressed,
	// 	&ThisClass::AbilityInputTagReleased,
	// 	&ThisClass::AbilityInputTagHeld);

	AuraInputComponent->BindAbilityActions(
		InputConfig, // 数据资产里配了 IA <-> Tag 表
		this,
		LrIMC,
		&ThisClass::AbilityInputTagPressed0,
		&ThisClass::AbilityInputTagReleased0,
		&ThisClass::AbilityInputTagHeld0);
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

	// 鼠标控制摄像机
	AuraInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALrPlayerController::Look);

	//跳
	// AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALrPlayerController::Jump);
}

void ALrPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateCamera(DeltaSeconds);
}

// 按下
// void ALrPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
// {
// 	if (LrASC == nullptr)
// 	{
// 		LrASC = Cast<ULrASC>(ULrCommonLibrary::GetASC(GetPawn()));
// 		if (LrASC == nullptr) return;
// 	}
// 	LrASC->AbilityInputTagPressed(InputTag);
// }
//
// // 释放
// void ALrPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
// {
// 	if (LrASC == nullptr)
// 	{
// 		LrASC = Cast<ULrASC>(ULrCommonLibrary::GetASC(GetPawn()));
// 		if (LrASC == nullptr) return;
// 	}
// 	//可能不触发ASC
// 	if (InputTag == FLrGameplayTags::Get().InputTag_Jump)
// 	{
// 		Jump();
// 	}
// 	else
// 	{
// 		LrASC->AbilityInputTagReleased(InputTag);
// 	}
// }
//
// // 一直按
// void ALrPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
// {
// 	if (LrASC == nullptr)
// 	{
// 		LrASC = Cast<ULrASC>(ULrCommonLibrary::GetASC(GetPawn()));
// 		if (LrASC == nullptr) return;
// 	}
// 	LrASC->AbilityInputTagHeld(InputTag);
// }

void ALrPlayerController::AbilityInputTagPressed0(int32 InputId)
{
	if (InhibitoryInput)
	{
		return;
	}
	if (LrASC == nullptr)
	{
		LrASC = Cast<ULrASC>(ULrCommonLibrary::GetASC(GetPawn()));
		if (LrASC == nullptr) return;
	}
	LrASC->AbilityInputTagPressed0(InputId);
}

void ALrPlayerController::AbilityInputTagReleased0(int32 InputId)
{
	if (InhibitoryInput)
	{
		return;
	}
	if (LrASC == nullptr)
	{
		LrASC = Cast<ULrASC>(ULrCommonLibrary::GetASC(GetPawn()));
		if (LrASC == nullptr) return;
	}
	//可能不触发ASC
	if (InputId == LrInputID::Jump)
	{
		Jump();
	}
	else
	{
		LrASC->AbilityInputTagReleased0(InputId);
	}
}

void ALrPlayerController::AbilityInputTagHeld0(int32 InputId)
{
	if (InhibitoryInput)
	{
		return;
	}
	if (LrASC == nullptr)
	{
		LrASC = Cast<ULrASC>(ULrCommonLibrary::GetASC(GetPawn()));
		if (LrASC == nullptr) return;
	}
	LrASC->AbilityInputTagHeld0(InputId);
}


void ALrPlayerController::Move(const FInputActionValue& InputActionValue)
{
	if (InhibitoryInput)
	{
		return;
	}
	// ULrCommonLibrary::PrintLog(GetWorld(), this);
	if (LrASC == nullptr)
	{
		TObjectPtr<APawn> tPawn = GetPawn();
		LrASC = Cast<ULrASC>(ULrCommonLibrary::GetASC(GetPawn()));
		if (LrASC == nullptr) return;
	}
	FVector Input;
	// 停止移动
	bool bIsBlockMove = LrASC->HasMatchingGameplayTag(FLrGameplayTags::Get().State_Block_Move);
	if (bIsBlockMove)
	{
		Input = FVector::ZeroVector;
		Input.Normalize();
	}
	else
	{
		// 增强输入默认返回 FVector2D
		// FVector2D InputVector = InputActionValue.Get<FVector2D>();
		// const FRotator Rotation = GetControlRotation();
		// const FRotator YawRotation(0, Rotation.Yaw, 0);
		// const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// Input = (ForwardDirection * InputVector.X) + (RightDirection * InputVector.Y);
		// Input.Normalize();

		// 增强输入默认返回 FVector2D
		FVector2D InputVector = InputActionValue.Get<FVector2D>();
		// 俯视角固定相机：移动方向始终基于相机的世界朝向，而非 ControlRotation
		// 这样无论 PlayerStart 的 Yaw 是多少，WASD 始终与屏幕上下左右一致
		const FRotator CameraRotation = PlayerCameraManager ? PlayerCameraManager->GetCameraRotation() : GetControlRotation();
		const FRotator YawRotation(0, CameraRotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		Input = (ForwardDirection * InputVector.X) + (RightDirection * InputVector.Y);
		Input.Normalize();
		//之前
		// Input = InputActionValue.Get<FVector>();
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
	const FVector Input = FVector::ZeroVector;
	if (ALrPawnBase* ControlledPawn = GetPawn<ALrPawnBase>())
	{
		ControlledPawn->UpdateMove(Input);
	}
}

void ALrPlayerController::Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	// 鼠标左右移动：水平旋转相机
	TargetCameraYaw += LookAxisVector.X * CameraYawSpeed;

	// 鼠标上下移动：调整俯仰角
	CameraPitch = FMath::Clamp(CameraPitch + LookAxisVector.Y * CameraPitchSpeed, MinCameraPitch, MaxCameraPitch);
}

void ALrPlayerController::Jump() const
{
	if (ALrPawnBase* ControlledPawn = GetPawn<ALrPawnBase>())
	{
		ControlledPawn->UpdatePressedJump(true);
	}
}

ALrPawnBase* ALrPlayerController::GetNearestPawnToCursor(float MaxScreenDistance)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	float MouseX;
	float MouseY;

	if (!GetMousePosition(MouseX, MouseY))
	{
		return nullptr;
	}

	const FVector2D MousePos(MouseX, MouseY);
	ALrPawnBase* BestPawn = nullptr;
	float BestDistanceSq = MaxScreenDistance * MaxScreenDistance;

	ALrPawnBase* MyPawn = Cast<ALrPawnBase>(GetPawn());
	for (TActorIterator<ALrPawnBase> It(World); It; ++It)
	{
		ALrPawnBase* TargetPawn = *It;

		if (!IsValid(TargetPawn))
		{
			continue;
		}

		//---------------------------------
		// 忽略自己
		//---------------------------------

		if (TargetPawn == MyPawn)
		{
			continue;
		}

		//---------------------------------
		// 投影到屏幕
		//---------------------------------

		FVector2D ScreenPos;
		// FVector WorldPos = TargetPawn->GetActorLocation();
		// if (UCapsuleComponent* Capsule = TargetPawn->FindComponentByClass<UCapsuleComponent>())
		// {
		// 	WorldPos.Z += Capsule->GetScaledCapsuleHalfHeight();
		// }

		bool bOnScreen = ProjectWorldLocationToScreen(TargetPawn->GetActorLocation(), ScreenPos);
		if (!bOnScreen)
		{
			continue;
		}

		//---------------------------------
		// 计算屏幕距离
		//---------------------------------
		float DistSq = FVector2D::DistSquared(MousePos, ScreenPos);
		if (DistSq < BestDistanceSq)
		{
			BestDistanceSq = DistSq;
			BestPawn = TargetPawn;
		}
	}

	// 切换选中
	if (CurrentSelectedPawn && BestPawn == CurrentSelectedPawn)
	{
		return BestPawn;
	}

	if (CurrentSelectedPawn)
	{
		CurrentSelectedPawn->SetSelected(false);
	}
	CurrentSelectedPawn = BestPawn;
	if (CurrentSelectedPawn)
	{
		CurrentSelectedPawn->SetSelected(true);
	}
	return BestPawn;
}

void ALrPlayerController::UpdateHoverTarget()
{
	GetNearestPawnToCursor();
}

void ALrPlayerController::UpdateCamera(float DeltaSeconds)
{
	ALrHeroPawn* Hero = Cast<ALrHeroPawn>(GetPawn());

	if (!Hero)
	{
		return;
	}

	if (!Hero->CameraBoom)
	{
		return;
	}

	//------------------------------------
	// 设置CameraBoom
	//------------------------------------

	CurrentCameraYaw = TargetCameraYaw;

	Hero->CameraBoom->SetWorldRotation(FRotator(
		CameraPitch,
		CurrentCameraYaw,
		0.f));
}
