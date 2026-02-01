// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/LrHUD.h"

#include "MVVMGameSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Interface/LrEquipInterface.h"
#include "UI/ViewModel/MVVMMainScreen.h"
#include "UI/Widget/LrMainWidget.h"


void ALrHUD::BeginPlay()
{
	Super::BeginPlay();
	// APlayerController* PC = GetOwningPlayerController();
	// if (!PC)
	// {
	// 	return;
	// }
	// 1️⃣ 创建 Widget
	MainWidget = CreateWidget<ULrMainWidget>(GetWorld(), MainWidgetClass);
	if (!MainWidget)return;

	// 2️⃣ 加到屏幕（关键！）
	MainWidget->AddToViewport();

	// 动态创建
	ViewModel = NewObject<UMVVMMainScreen>(GetTransientPackage(), FName("UMVVMMainScreen"));
	// 创建 ViewModel（HUD 持有）
	// ViewModel = NewObject<UMVVMMainScreen>(this, ViewModelClass);
	// 监听 ViewModel 请求
	ViewModel->OnEquipRequest.AddUObject(this, &ALrHUD::HandleEquipRequest);
	ViewModel->OnUnequipRequest.AddUObject(this, &ALrHUD::HandleUnequipRequest);

	// UMVVMGameSubsystem* MVVMGameSubsystem = GetGameInstance()->GetSubsystem<UMVVMGameSubsystem>();
	// UMVVMViewModelCollectionObject* Collection = MVVMGameSubsystem->GetViewModelCollection();
	// FMVVMViewModelContext Context;
	// Context.ContextClass = UMVVMMainScreen::StaticClass();
	// Context.ContextName = TEXT("MVVMMainScreen"); // 和 Widget Designer 完全一致
	// bool bAdded = Collection->AddViewModelInstance(Context, ViewModel);
	// check(bAdded);
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
