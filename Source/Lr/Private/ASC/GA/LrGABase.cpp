// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GA/LrGABase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ASC/AS/LrAS.h"
#include "GameplayEffect.h"
#include "Tags/LrGameplayTags.h"

bool ULrGABase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                   FGameplayTagContainer* OptionalRelevantTags) const
{
	if (IsActive())
	{
		return false;
	}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

bool ULrGABase::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	if (!HasValidCooldownConfig() || !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return true;
	}

	float RemainingTime = 0.f;
	float TotalDuration = 0.f;
	const bool bInCooldown = GetCooldownRemainingFromASC(ActorInfo->AbilitySystemComponent.Get(), RemainingTime, TotalDuration);
	return !bInCooldown;
}

void ULrGABase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!DefaultCooldownGE || !HasValidCooldownConfig())
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DefaultCooldownGE, GetAbilityLevel(Handle, ActorInfo));
	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		return;
	}

	// 同一个 GE 模板复用给多个技能时，真正的冷却时长由 SetByCaller 在运行时注入。
	SpecHandle.Data->SetSetByCallerMagnitude(FLrGameplayTags::Get().Data_Cooldown, AbilityConfig.Cooldown.CooldownDuration);
	// 这里不用 DynamicGrantedTags。
	// 原因是共享同一个 GE 且 GE 发生叠层时，UE 要求旧/新 Spec 的 DynamicGrantedTags 完全一致，否则会触发 ensure。
	// 改用 DynamicAssetTag 仅作为“这是哪个技能冷却”的标识，再由自定义查询判断冷却。
	SpecHandle.Data->AddDynamicAssetTag(AbilityConfig.Cooldown.CooldownTag);

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}

bool ULrGABase::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid() || !HasAnyCost())
	{
		return true;
	}

	// 统一从 AbilityConfig 读取资源消耗，避免每个技能各写一套校验。
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const FLrAbilityCostConfig& CostConfig = AbilityConfig.Cost;

	return CheckAttributeCost(ASC, ULrAS::GetMPAttribute(), CostConfig.Mana)
		&& CheckAttributeCost(ASC, ULrAS::GetEnduranceAttribute(), CostConfig.Stamina)
		&& CheckAttributeCost(ASC, ULrAS::GetHPAttribute(), CostConfig.HP);
}

void ULrGABase::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid() || !HasAnyCost())
	{
		return;
	}

	// 直接按属性扣减资源，当前实现不额外走 Cost GE。
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const FLrAbilityCostConfig& CostConfig = AbilityConfig.Cost;

	ApplyAttributeCost(ASC, ULrAS::GetMPAttribute(), CostConfig.Mana);
	ApplyAttributeCost(ASC, ULrAS::GetEnduranceAttribute(), CostConfig.Stamina);
	ApplyAttributeCost(ASC, ULrAS::GetHPAttribute(), CostConfig.HP);
}

const FGameplayTagContainer* ULrGABase::GetCooldownTags() const
{
	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();
	if (!AbilityConfig.Cooldown.CooldownTag.IsValid())
	{
		return ParentTags;
	}

	// 把父类冷却 Tag 和当前技能自己的冷却 Tag 合并后返回给 GAS。
	CachedCooldownTags.Reset();
	if (ParentTags)
	{
		CachedCooldownTags.AppendTags(*ParentTags);
	}
	CachedCooldownTags.AddTag(AbilityConfig.Cooldown.CooldownTag);

	return &CachedCooldownTags;
}

UGameplayEffect* ULrGABase::GetCooldownGameplayEffect() const
{
	if (!DefaultCooldownGE)
	{
		return nullptr;
	}

	return DefaultCooldownGE->GetDefaultObject<UGameplayEffect>();
}

bool ULrGABase::GetCooldownRemainingForActor(AActor* AvatarActor, float& OutRemainingTime, float& OutTotalDuration) const
{
	OutRemainingTime = 0.f;
	OutTotalDuration = 0.f;

	if (!AvatarActor)
	{
		return false;
	}

	// UI 通常拿到的是角色/Avatar，这里转成 ASC 后复用底层查询逻辑。
	const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AvatarActor);
	return GetCooldownRemainingFromASC(ASC, OutRemainingTime, OutTotalDuration);
}

bool ULrGABase::IsAbilityInCooldownForActor(AActor* AvatarActor) const
{
	float RemainingTime = 0.f;
	float TotalDuration = 0.f;
	return GetCooldownRemainingForActor(AvatarActor, RemainingTime, TotalDuration) && RemainingTime > 0.f;
}

float ULrGABase::GetConfiguredCooldownDuration() const
{
	return AbilityConfig.Cooldown.CooldownDuration;
}

bool ULrGABase::HasValidCooldownConfig() const
{
	return AbilityConfig.Cooldown.CooldownTag.IsValid() && AbilityConfig.Cooldown.CooldownDuration > 0.f;
}

bool ULrGABase::HasAnyCost() const
{
	return AbilityConfig.Cost.Mana > 0.f || AbilityConfig.Cost.Stamina > 0.f || AbilityConfig.Cost.HP > 0.f;
}

bool ULrGABase::CheckAttributeCost(const UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute, float Cost) const
{
	if (!ASC || !Attribute.IsValid() || Cost <= 0.f)
	{
		return true;
	}

	// 只做简单的“当前值是否足够”判断。
	return ASC->GetNumericAttribute(Attribute) >= Cost;
}

void ULrGABase::ApplyAttributeCost(UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute, float Cost) const
{
	if (!ASC || !Attribute.IsValid() || Cost <= 0.f)
	{
		return;
	}

	// 直接改 BaseValue，让 AttributeSet 的夹取逻辑继续生效。
	const float CurrentValue = ASC->GetNumericAttribute(Attribute);
	ASC->SetNumericAttributeBase(Attribute, CurrentValue - Cost);
}

bool ULrGABase::GetCooldownRemainingFromASC(const UAbilitySystemComponent* ASC, float& OutRemainingTime, float& OutTotalDuration) const
{
	OutRemainingTime = 0.f;
	OutTotalDuration = 0.f;

	if (!ASC || !AbilityConfig.Cooldown.CooldownTag.IsValid())
	{
		return false;
	}

	// 冷却本质上就是 ASC 上某个带冷却 Tag 的活动 GE，UI 查询时按 Tag 反查即可。
	FGameplayTagContainer CooldownTagContainer;
	CooldownTagContainer.AddTag(AbilityConfig.Cooldown.CooldownTag);

	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(CooldownTagContainer);
	const TArray<TPair<float, float>> TimePairs = ASC->GetActiveEffectsTimeRemainingAndDuration(Query);
	if (TimePairs.IsEmpty())
	{
		return false;
	}

	for (const TPair<float, float>& TimePair : TimePairs)
	{
		// 理论上同一 Tag 下可能存在多个效果，这里取剩余时间最长的那个作为当前冷却。
		if (TimePair.Key > OutRemainingTime)
		{
			OutRemainingTime = TimePair.Key;
			OutTotalDuration = TimePair.Value;
		}
	}

	return OutRemainingTime > 0.f;
}
