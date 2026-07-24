// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Crosshair/LrCrosshairWidget.h"

#include "Components/Image.h"

void ULrCrosshairWidget::SetTargetState(bool bTarget)
{
	if (bHasTarget == bTarget)
	{
		return;
	}
	
	bHasTarget = bTarget;
	if (!AnimTargetRef)
	{
		return;
	}

	if (bHasTarget)
	{
		PlayAnimation(AnimTargetRef, 0.f, 1, EUMGSequencePlayMode::Forward);
	}
	else
	{
		PlayAnimation(AnimTargetRef, 0.f, 1, EUMGSequencePlayMode::Reverse);
	}
}

void ULrCrosshairWidget::SetCrosshairVisible(bool bVisible)
{
	if (bVisible)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}
