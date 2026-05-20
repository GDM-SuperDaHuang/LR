// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/LrHUD.h"

#include "MVVMGameSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Interface/LrEquipInterface.h"
#include "Pawn/LrPawnBase.h"
#include "UI/UIController/LrUIController.h"
#include "UI/ViewModel/MVVMMainScreen.h"
#include "UI/Widget/LrMainWidget.h"
#include "UI/Widget/Bar/LrHPBarWidget.h"
#include "UI/Widget/Bar/LrMPBarWidget.h"


void ALrHUD::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}
	ALrPawnBase* LrPawnBase = Cast<ALrPawnBase>(PC->GetPawn());
	
	UIController = NewObject<ULrUIController>(this);
	UIController->Init(LrPawnBase->LrASComponent);

	// 添加到屏幕
	ULrHPBarWidget* HPWidget = CreateWidget<ULrHPBarWidget>(GetWorld(),HPWidgetClass.Get());
	HPWidget->SetViewModel(UIController->HPVM);
	// HPWidget->AddToViewport();

	ULrMPBarWidget* MPWidget =CreateWidget<ULrMPBarWidget>(GetWorld(),MPWidgetClass.Get());
	MPWidget->SetViewModel(UIController->MPVM);
	// MPWidget->AddToViewport();

	
	
	// 1️⃣ 创建 Widget
	MainWidget = CreateWidget<ULrMainWidget>(GetWorld(), MainWidgetClass);
	if (!MainWidget)return;

	// 2️⃣ 加到屏幕（关键！）
	MainWidget->AddToViewport();

	// 动态创建 GetTransientPackage()
	ViewModel = NewObject<UMVVMMainScreen>(GetWorld(), FName("UMVVMMainScreen"));
	// 创建 ViewModel（HUD 持有）
	// ViewModel = NewObject<UMVVMMainScreen>(this, ViewModelClass);
	// 监听 ViewModel 请求
	ViewModel->OnEquipRequest.AddUObject(this, &ALrHUD::HandleEquipRequest);
	ViewModel->OnUnequipRequest.AddUObject(this, &ALrHUD::HandleUnequipRequest);
	
}

void ALrHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (UIController)
	{
		UIController->Tick(DeltaSeconds);
	}
}


void ALrHUD::HandleEquipRequest(FLrWeaponConfig WeaponConfig)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	// 最终执行装备
	if (ILrEquipInterface* EquipTarget = Cast<ILrEquipInterface>(Pawn))
	{
		EquipTarget->EquipWeapon(WeaponConfig);
	}
}

void ALrHUD::HandleUnequipRequest(FLrWeaponConfig WeaponConfig)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	if (ILrEquipInterface* EquipTarget = Cast<ILrEquipInterface>(Pawn))
	{
		EquipTarget->Unequipped(WeaponConfig);
	}
}
