// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LrSkillPanelWidget.generated.h"

class UHorizontalBox;
class ULrSkillWidget;
class ULrMVVMSkillSlot;

UCLASS()
class LR_API ULrSkillPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetSkillSlotViewModels(const TArray<ULrMVVMSkillSlot*>& InViewModels);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TSubclassOf<ULrSkillWidget> SkillSlotWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UHorizontalBox> SkillSlotContainer;

private:
	UPROPERTY()
	TArray<TObjectPtr<ULrSkillWidget>> SkillSlotWidgets;
};