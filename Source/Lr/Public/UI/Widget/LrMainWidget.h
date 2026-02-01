// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LrMainWidget.generated.h"

class ULrButtonInfo;
class UMVVMViewModelBase;
class UMVVMMainScreen;
/**
 * 
 */
UCLASS()
class LR_API ULrMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 在 Widget Blueprint 中绑定的 Button（必须在 Designer 中命名一致，或用 BindWidget）
	// UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UI")
	// ULrButtonInfo* BtnWear; // 蓝图的按钮名称必须是叫 BtnWear，并且必须要有按钮

	// 可选：如果你想在 C++ 中创建 ViewModel（通常在 BP 中创建更方便）
	UPROPERTY()
	UMVVMMainScreen* MainViewModel;

	UFUNCTION()
	void OnAnyWearButtonClicked(ULrButtonInfo* Button);

	// 第一次被添加到 UI 层次结构并准备好进行交互/渲染时调用的
	virtual void NativeConstruct() override;

protected:
	// 点击事件的处理函数（推荐放在 Widget 中，作为桥接层）
	// UFUNCTION()
	// void OnWearButtonClicked();
};
