// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LrMainWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Const/LrConst.h"
#include "Lib/LrCommonLibrary.h"
#include "UI/HUD/LrHUD.h"

#include "UI/ViewModel/MVVMMainScreen.h"
#include "UI/Widget/Button/LrButtonInfo.h"
#include "View/MVVMView.h"

void ULrMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 找到当前 Widget 里的所有 ULrButtonInfo
	TArray<UWidget*> Widgets;
	WidgetTree->GetAllWidgets(Widgets);

	for (UWidget* Widget : Widgets)
	{
		if (ULrButtonInfo* Btn = Cast<ULrButtonInfo>(Widget))
		{
			Btn->OnButtonInfoClicked.AddDynamic(
				this,
				&ULrMainWidget::OnAnyWearButtonClicked
			);
		}
	}

	// if (BtnWear)
	// {
	// 	// 使用 AddDynamic 动态绑定（这是 UButton OnClicked 的标准用法）
	// 	BtnWear->OnClicked.AddDynamic(this, &ULrMainWidget::OnWearButtonClicked);
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("BtnWear 未绑定！请在 Widget Blueprint 中命名 Button 为 BtnWear"));
	// }
}

void ULrMainWidget::OnAnyWearButtonClicked(ULrButtonInfo* Button)
{
	if (MainViewModel == nullptr)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (ALrHUD* aLrHUD = Cast<ALrHUD>(PC->GetHUD()))
			{
				MainViewModel = aLrHUD->ViewModel;
			}
		}
	}

	if (MainViewModel)
	{
		// 调用 ViewModel 的“穿上”逻辑
		FLrWeaponConfig WeaponConfig = ULrCommonLibrary::FindWeaponByID(this, Button->WeaponID);

		if (Button->BehaviorID == Weapon::Equipped)
		{
			MainViewModel->EquipWeapon(WeaponConfig);
			UE_LOG(LogTemp, Log, TEXT("穿上按钮被点击，已调用 ViewModel 的 WearItem()"));
		}
		else if (Button->BehaviorID == Weapon::Unequipped)
		{
			MainViewModel->Unequipped(WeaponConfig);
			UE_LOG(LogTemp, Log, TEXT("穿上按钮被点击，已调用 ViewModel 的 WearItem()"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ViewModel 未设置！"));
	}
}
