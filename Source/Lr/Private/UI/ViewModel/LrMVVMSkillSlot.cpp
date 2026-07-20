// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/LrMVVMSkillSlot.h"

#include "ASC/GA/LrGABase.h"

void ULrMVVMSkillSlot::Initialize(ULrGABase* InAbility, AActor* InOwnerActor, int32 InInputID)
{
	Ability = InAbility;
	OwnerActor = InOwnerActor;
	InputID = InInputID;

	if (Ability)
	{
		SkillIcon = Ability->AbilityConfig.SkillIcon;
		CooldownTotal = Ability->GetConfiguredCooldownDuration();
	}
}

void ULrMVVMSkillSlot::Tick(float DeltaTime)
{
	if (!Ability || !OwnerActor.IsValid())
	{
		return;
	}

	float Remaining = 0.f;
	float Total = 0.f;
	const bool bWasInCooldown = bInCooldown;
	bInCooldown = Ability->GetCooldownRemainingForActor(OwnerActor.Get(), Remaining, Total);

	if (Remaining < 0.f)
	{
		Remaining = 0.f;
	}

	CooldownRemaining = Remaining;
	if (Total > 0.f)
	{
		CooldownTotal = Total;
	}
	CooldownPercent = CooldownTotal > 0.f ? CooldownRemaining / CooldownTotal : 0.f;

	if (CooldownRemaining > 0.f)
	{
		CooldownText = FText::AsNumber(FMath::CeilToInt(CooldownRemaining));
	}
	else
	{
		CooldownText = FText::GetEmpty();
	}

	if (bInCooldown != bWasInCooldown)
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bInCooldown);
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CooldownRemaining);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CooldownTotal);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CooldownPercent);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CooldownText);
}