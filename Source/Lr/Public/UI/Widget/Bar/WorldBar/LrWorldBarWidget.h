// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LrWorldBarWidget.generated.h"

struct FGameplayTag;
class ALrEnemyPawn;
class UTextBlock;
class UProgressBar;
/**
 * 敌人UI血条
 */
UCLASS()
class LR_API ULrWorldBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitWidget(ALrEnemyPawn* InEnemy);
	void UpdateChance(FGameplayTag ASTag, float Current, float Max) const;

	void UpdateHealth(float Current, float Max) const;
	virtual void NativeConstruct() override;
	void PlayDamageAnimation(float DamageValue);

protected:
	// 主进度条
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UProgressBar> MainBar;
	// 虚幻进度条
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UProgressBar> GhostBar;

	UPROPERTY()
	TObjectPtr<ALrEnemyPawn> OwnerEnemy;

	// 数值显示
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> TextHP;

	// 自动绑定蓝图中的 UI 动画，变量名必须和蓝图里的动画名【完全一致】
	// 注意：动画必须加上 Transient
	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim, AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetAnimation> DamageAnimRef;
};
