// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LrCrosshairWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class LR_API ULrCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetTargetState(bool bTarget);
	void SetCrosshairVisible(bool bVisible);
protected:
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CrosshairImage;
	
	// 自动绑定蓝图中的 UI 动画，变量名必须和蓝图里的动画名【完全一致】
	// 注意：动画必须加上 Transient
	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim, AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetAnimation> AnimTargetRef;
	
private:
	bool bHasTarget=false;

};
