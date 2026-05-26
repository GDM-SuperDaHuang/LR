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

	void UpdateHealth(FGameplayTag GameplayTag, float Percent) const;
	virtual void NativeConstruct() override;

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
};
