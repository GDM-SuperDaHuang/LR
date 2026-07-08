// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GA/LrGABase.h"

#include "Tags/LrGameplayTags.h"

void ULrGABase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!DefaultCooldownGE)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
		DefaultCooldownGE,
		GetAbilityLevel()
	);

	SpecHandle.Data->SetSetByCallerMagnitude(FLrGameplayTags::Get().Data_Cooldown, CooldownDuration);
	// 把技能的冷却标签加到这个 GE
	SpecHandle.Data->DynamicGrantedTags.AppendTags(CooldownTags);

	// 把技能的冷却标签加到这个 GE
	FActiveGameplayEffectHandle GameplayEffectSpecToOwner = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}

const FGameplayTagContainer* ULrGABase::GetCooldownTags() const
{
	return &CooldownTags;
}

UGameplayEffect* ULrGABase::GetCooldownGameplayEffect() const
{
	if (!DefaultCooldownGE)
	{
		return nullptr;
	}
	return DefaultCooldownGE->GetDefaultObject<UGameplayEffect>();
}
