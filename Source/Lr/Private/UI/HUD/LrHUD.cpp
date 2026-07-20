// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/LrHUD.h"

#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "ASC/LrASC.h"
#include "ASC/AS/LrAS.h"
#include "Blueprint/UserWidget.h"
#include "Interface/LrEquipInterface.h"
#include "Pawn/LrPawnBase.h"
#include "UI/UIController/LrUIController.h"
#include "UI/ViewModel/MVVMMainScreen.h"
#include "UI/Widget/LrMainWidget.h"
#include "UI/Widget/Bar/LrHPBarWidget.h"
#include "UI/Widget/Bar/LrMPBarWidget.h"
#include "UI/Widget/Skill/LrSkillPanelWidget.h"


void ALrHUD::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}
	ALrPawnBase* LrPawnBase = Cast<ALrPawnBase>(PC->GetPawn());
	if (!LrPawnBase)
	{
		return;
	}
	UIController = NewObject<ULrUIController>(this);
	UIController->Init();

	// 添加到屏幕
	// ULrHPBarWidget* HPWidget = CreateWidget<ULrHPBarWidget>(GetWorld(), HPWidgetClass.Get());
	// HPWidget->SetViewModel(UIController->HPVM);
	// UE_LOG(LogTemp, Warning, TEXT("MVVM: %p"), UIController->HPVM.Get());


	// ULrMPBarWidget* MPWidget = CreateWidget<ULrMPBarWidget>(GetWorld(), MPWidgetClass.Get());
	// MPWidget->SetViewModel(UIController->MPVM);
	// UE_LOG(LogTemp, Warning, TEXT("MVVM: %p"), UIController->MPVM.Get());

	// 1️⃣ 创建 Widget
	MainWidget = CreateWidget<ULrMainWidget>(GetWorld(), MainWidgetClass);
	if (!MainWidget)return;
	// 直接给自动创建好的子 UI 绑定 ViewModel
	MainWidget->HPBarWidget->SetViewModel(TEXT("HPVM"), UIController->HPVM);
	MainWidget->MPBarWidget->SetViewModel(TEXT("MPVM"), UIController->MPVM);

	// 2️⃣ 加到屏幕（关键！）
	MainWidget->AddToViewport();


	// 动态创建 GetTransientPackage()
	ViewModel = NewObject<UMVVMMainScreen>(GetWorld(), FName("UMVVMMainScreen"));
	// 创建 ViewModel（HUD 持有）
	// ViewModel = NewObject<UMVVMMainScreen>(this, ViewModelClass);
	// 监听 ViewModel 请求
	ViewModel->OnEquipRequest.AddUObject(this, &ALrHUD::HandleEquipRequest);
	ViewModel->OnUnequipRequest.AddUObject(this, &ALrHUD::HandleUnequipRequest);

	// 获取监听的 GAS 属性
	// 可能没来及注册
	if (LrPawnBase->GetASC())
	{
		// 如果已经错过了广播，我们在这里手动补救调用
		HandleASCRegistered(LrPawnBase->GetASC());
	}
	LrPawnBase->OnASCRegistered.AddUObject(this, &ALrHUD::HandleASCRegistered);
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

void ALrHUD::HandleASCRegistered(ULrASC* LrAsc)
{
	APlayerController* PC = GetOwningPlayerController();
	ALrPawnBase* LrPawnBase = Cast<ALrPawnBase>(PC->GetPawn());
	ULrAS* LrAs = LrPawnBase->GetAS();
	for (TPair<FGameplayTag, FGameplayAttribute(*)()> Pair : LrAs->TagsASMap)
	{
		LrAsc->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this,Pair,LrAs](const FOnAttributeChangeData& Data)
			{
				const FGameplayTag* TagsAsMaxTag = LrAs->TagsASMaxTags.Find(Pair.Key);
				float Max = 0;
				if (TagsAsMaxTag)
				{
					FAttributeFuncPtr* Find = LrAs->TagsASMap.Find(*TagsAsMaxTag);
					if (Find)
					{
						FGameplayAttribute MaxAttribute = (*Find)();
						Max = MaxAttribute.GetNumericValue(LrAs);
					}
				}
				UIController->OnASChanged(Pair.Key, Pair.Value().GetNumericValue(LrAs), Max);
			});
	}

	if (UIController && MainWidget && MainWidget->SkillPanelWidget)
	{
		UIController->InitSkillSlots(LrPawnBase);
		MainWidget->SkillPanelWidget->SetSkillSlotViewModels(UIController->SkillSlotVMs);
	}

	LrPawnBase->InitAS();
	// LrPawnBase->OnASCRegistered.RemoveDynamic(LrAsc);
}
