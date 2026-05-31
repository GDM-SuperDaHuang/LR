// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/LrTickableWorldSubsystem.h"

#include "UI/Widget/Bar/WorldBar/LrWorldBarWidget.h"

TStatId ULrTickableWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULrWorldBarSubsystem, STATGROUP_Tickables);
}

void ULrTickableWorldSubsystem::Tick(float DeltaTime)
{
	for (int32 i = ActiveBars.Num() - 1; i >= 0; --i)
	{
		ULrWorldBarWidget* Bar = ActiveBars[i].Get();
		if (!Bar)
		{
			ActiveBars.RemoveAtSwap(i);
			Bar->bIsAnimating = false;
			continue;
		}
		Bar->Tick(DeltaTime);
		if (FMath::IsNearlyEqual(Bar->LastGhostPercent, Bar->TargetPercent, 0.001f))
		{
			ActiveBars.RemoveAtSwap(i);
			Bar->bIsAnimating = false;
		}
	}
}

void ULrTickableWorldSubsystem::RegisterActiveBar(ULrWorldBarWidget* Bar)
{
	if (!Bar)
	{
		return;
	}
	if (Bar->bIsAnimating)
	{
		return;
	}
	ActiveBars.Add(Bar);
	Bar->bIsAnimating = true;
}
