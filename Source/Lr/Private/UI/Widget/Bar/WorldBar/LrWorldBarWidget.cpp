// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Bar/WorldBar/LrWorldBarWidget.h"

#include "GameplayTagContainer.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ULrWorldBarWidget::InitWidget(ALrEnemyPawn* InEnemy)
{
	OwnerEnemy = InEnemy;
}

void ULrWorldBarWidget::UpdateHealth(FGameplayTag GameplayTag,float Percent) const
{
	if (MainBar)
	{
		MainBar->SetPercent(Percent);
	}

	if (TextHP)
	{
		TextHP->SetText(FText::AsNumber(FMath::RoundToInt(Percent * 100.f)));
	}
}

void ULrWorldBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	check(MainBar);
	check(TextHP);
}