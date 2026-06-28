// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/LrTickableWorldSubsystem.h"

#include "Actor/Lightning/LrLightning.h"
#include "Actor/Projectile/LrProjectile.h"
#include "Lib/LrCommonLibrary.h"
#include "Pawn/LrPawnBase.h"
#include "UI/Widget/Bar/WorldBar/LrWorldBarWidget.h"

TStatId ULrTickableWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULrWorldBarSubsystem, STATGROUP_Tickables);
}

/**
 * 集中刷新
 * @param DeltaTime 
 */
void ULrTickableWorldSubsystem::Tick(float DeltaTime)
{
	for (int32 i = ActiveBars.Num() - 1; i >= 0; --i)
	{
		ULrWorldBarWidget* Bar = ActiveBars[i].Get();
		if (!Bar)
		{
			ActiveBars.RemoveAtSwap(i);
			continue;
		}
		Bar->Tick(DeltaTime);
		if (FMath::IsNearlyEqual(Bar->LastGhostPercent, Bar->TargetPercent, 0.001f))
		{
			if (FMath::IsNearlyEqual(Bar->LastGhostPercent, 0.f, 0.001f))
			{
				ActiveBars.RemoveAtSwap(i);
			}
			Bar->bIsAnimating = false;
		}
		else
		{
			Bar->bIsAnimating = true;
		}
	}
}

void ULrTickableWorldSubsystem::RegisterActiveBar(ULrWorldBarWidget* Bar)
{
	ActiveBars.AddUnique(Bar);
}

void ULrTickableWorldSubsystem::RemoveLightning(const ALrPawnBase* ALrPawnBas)
{
	LrLightningPool.Remove(ALrPawnBas);
}

ALrLightning* ULrTickableWorldSubsystem::GetLightning(ALrPawnBase* LrPawnBas)
{
	ALrLightning** LrLightning = LrLightningPool.Find(LrPawnBas);
	if (LrLightning == nullptr)
	{
		return RegisterLightning(LrPawnBas);
	}
	if (*LrLightning)
	{
		return *LrLightning;
	}
	return nullptr;
}

ALrLightning* ULrTickableWorldSubsystem::RegisterLightning(ALrPawnBase* LrPawnBase)
{
	// todo 从对象池获取
	// FActorSpawnParameters Params;
	// Params.Owner = ALrPawnBas;
	// Params.Instigator = ALrPawnBas;
	if (ALrGameModeBase* LrGameModeBase = ULrCommonLibrary::GetLrGameModeBase(LrPawnBase))
	{
		ULrGAListDA* LrGaListDa = LrGameModeBase->LrGAListDA;
		ALrLightning* LrLightning = GetWorld()->SpawnActor<ALrLightning>(LrGaListDa->LightningClass, LrPawnBase->GetActorTransform(), FActorSpawnParameters());
		LrLightning->Init(LrPawnBase);
		LrLightningPool.Add(LrPawnBase, LrLightning);
		return LrLightning;
	}
	return nullptr;
}
