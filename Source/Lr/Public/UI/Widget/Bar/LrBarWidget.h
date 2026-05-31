// Fill out your copyright notice in the Description page of Project Settings.

/**
 * 不存业务状态
Widget 不应该：
获取 Character
获取 Attribute
Tick 算逻辑
监听 Gameplay
 */
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LrBarWidget.generated.h"

class UTextBlock;
class UProgressBar;
class ULrMVVMVBar;
/**
 * 通用资源条控件，支持：
 * - 主进度条（实时变化）
 * - 幻影进度条（平滑跟随）
 * - 可配置追赶速度 
 */
UCLASS()
class LR_API ULrBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetViewModel(FName ViewModelName,ULrMVVMVBar* InViewModel);
protected:
	// // 主进度条
	// UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	// TObjectPtr<UProgressBar> MainBar;
	//
	// // 虚幻进度条
	// UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	// TObjectPtr<UProgressBar> GhostBar;
	//
	// // 数值显示
	// UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	// TObjectPtr<UTextBlock> TextHP;

	// VM
	UPROPERTY()
	TObjectPtr<ULrMVVMVBar> ViewModel;
};
