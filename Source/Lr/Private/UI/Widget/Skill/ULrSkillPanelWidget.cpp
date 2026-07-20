// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Skill/LrSkillPanelWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "UI/ViewModel/LrMVVMSkillSlot.h"
#include "UI/Widget/Skill/LrSkillWidget.h"

void ULrSkillPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void ULrSkillPanelWidget::SetSkillSlotViewModels(const TArray<ULrMVVMSkillSlot*>& InViewModels)
{
	if (!SkillSlotContainer)
	{
		return;
	}

	SkillSlotContainer->ClearChildren();
	SkillSlotWidgets.Empty();

	for (ULrMVVMSkillSlot* SkillSlotVM : InViewModels)
	{
		if (!SkillSlotVM || !SkillSlotWidgetClass)
		{
			continue;
		}

		ULrSkillWidget* SkillWidget = CreateWidget<ULrSkillWidget>(GetWorld(), SkillSlotWidgetClass);
		if (!SkillWidget)
		{
			continue;
		}

		SkillWidget->SetViewModel(TEXT("SkillSlotVM"), SkillSlotVM);

		if (UHorizontalBoxSlot* BoxSlot = SkillSlotContainer->AddChildToHorizontalBox(SkillWidget))
		{
			BoxSlot->SetPadding(FMargin(8.f, 0.f));
			BoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		}

		SkillSlotWidgets.Add(SkillWidget);
	}
}