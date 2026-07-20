// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Skill/LrSkillWidget.h"

#include "MVVMSubsystem.h"
#include "View/MVVMView.h"
#include "UI/ViewModel/LrMVVMSkillSlot.h"

void ULrSkillWidget::SetViewModel(FName ViewModelName, ULrMVVMSkillSlot* InViewModel)
{
	ViewModel = InViewModel;

	if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(this))
	{
		TScriptInterface<INotifyFieldValueChanged> NotifyFieldValueChanged = TScriptInterface<INotifyFieldValueChanged>(InViewModel);
		View->SetViewModel(ViewModelName, NotifyFieldValueChanged);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("无法获取 MVVMView！请确认该 Widget 在蓝图中启用了 MVVM 插件并添加了对应的 ViewModel！"));
	}
}

void ULrSkillWidget::NativeConstruct()
{
	Super::NativeConstruct();
}