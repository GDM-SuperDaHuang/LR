// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Bar/WorldBar/LrWorldBarWidget.h"

#include "GameplayTagContainer.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Game/LrTickableWorldSubsystem.h"
#include "Tags/LrGameplayTags.h"

// void ULrWorldBarWidget::InitWidget(ALrEnemyPawn* InEnemy)
// {
// 	OwnerEnemy = InEnemy;
// }

void ULrWorldBarWidget::UpdateChance(FGameplayTag ASTag, float Current, float Max) 
{
	FLrGameplayTags LrTags = FLrGameplayTags::Get();
	if (ASTag == LrTags.As_HP)
	{
		UpdateHealth(Current, Max);
	}
}


void ULrWorldBarWidget::UpdateHealth(float Current, float Max)
{
	TargetPercent = Max <= 0.f ? 0.f : Current / Max;
	if (MainBar)
	{
		MainBar->SetPercent(TargetPercent);
	}
	if (ULrTickableWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULrTickableWorldSubsystem>())
	{
		Subsystem->RegisterActiveBar(this);
	}
}

void ULrWorldBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	check(MainBar);
	check(TextHP);
}


void ULrWorldBarWidget::PlayDamageAnimation(float DamageValue)
{
	// 1. 设置伤害数字
	if (TextHP)
	{
		TextHP->SetText(FText::AsNumber(DamageValue));
	}

	// 2. 播放 UI 动画
	if (DamageAnimRef)
	{
		// 参数依次为：动画指针、开始时间、循环次数、播放模式、播放速度、是否恢复原状
		PlayAnimation(DamageAnimRef, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, false);
	}
}

void ULrWorldBarWidget::Tick(float DeltaTime)
{
	const float NewGhost = FMath::FInterpTo(LastGhostPercent, TargetPercent, DeltaTime, InterpSpeed);
	if (GhostBar)
	{
		GhostBar->SetPercent(NewGhost);
	}
	LastGhostPercent = NewGhost;
}
