// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Bar/LrBarWidget.h"
#include "UI/ViewModel/LrMVVMVBar.h" // 👈 必须包含这个，让编译器看到完整的类继承结构！
#include "MVVMSubsystem.h"
#include "View/MVVMView.h"


void ULrBarWidget::SetViewModel(FName ViewModelName,ULrMVVMVBar* InViewModel)
{
	ViewModel = InViewModel;

	// 2. 获取该 Widget 的 MVVM View 扩展
	if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(this))
	{
		// 3. 将 ViewModel 注入到系统中
		// 注意："ViewModel" 是你在 UMG 设计器（Viewmodels 面板）中给这个 VM 起的变量名 (Name)
		// 如果你在蓝图里起的名字叫 "HPVM"，这里就要写 TEXT("HPVM")
		TScriptInterface<INotifyFieldValueChanged> NotifyFieldValueChanged = TScriptInterface<INotifyFieldValueChanged>(InViewModel);
		View->SetViewModel(ViewModelName, NotifyFieldValueChanged);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("无法获取 MVVMView！请确认该 Widget 在蓝图中启用了 MVVM 插件并添加了对应的 ViewModel！"));
	}
}

