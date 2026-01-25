// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/LrASC.h"

#include "AbilitySystemGlobals.h"
#include "ASC/GA/LrGABase.h"
#include "Data/LrGAListDA.h"
#include "Lib/LrCommonLibrary.h"
#include "Tags/LrGameplayTags.h"

void ULrASC::AddGA(const TArray<FGameplayTag>& GATagList)
{
	for (const FGameplayTag GATag : GATagList)
	{
		FLrDAConfig LrDAConfig = ULrCommonLibrary::FindGAByTag(GetOwner(), GATag);
		// 创建技能规格（Spec），Level 1，默认不给予输入
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(LrDAConfig.GAClass, 1);
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(LrDAConfig.InputTag);
		/**
		* GiveAbility
		* 1. 生成全局唯一 Handle （通常一种技能一个）
		* 2. 实例化 GA 对象（CDO → 实例），里面初始化了AvatarActor ，OwnerActor，PlayerController 
		* 3. 塞进 ActiveAbilitySpecs 数组
		* 4. 标记网络脏，客户端会收到复制包并本地生成影子 Spec
		*/
		GiveAbility(AbilitySpec); // 真正交给 ASC 管理
		UE_LOG(LogTemp, Warning,TEXT("Client Ability Count: %d"),GetActivatableAbilities().Num());
		//  todo 技能初始化广播 ？？
		// bStartUpAbilitiesGiven = true;
		// AbilitiesGivenDelegate.Broadcast();
	}
}

void ULrASC::AbilityInputTagPressed(const FGameplayTag& InputTags)
{
	
	if (!InputTags.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	TArray<FGameplayAbilitySpec> GameplayAbilitySpecs = GetActivatableAbilities();
	for (FGameplayAbilitySpec AbilitySpec : GameplayAbilitySpecs)
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTag(InputTags))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (AbilitySpec.IsActive())
			{
				// todo 
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}

/**
 * 输入系统回调：某输入标签被“按住”时调用
 * 遍历所有已拥有的技能，找到 DynamicAbilityTags 匹配的标签 → 尝试激活
 */
void ULrASC::AbilityInputTagHeld(const FGameplayTag& InputTags)
{
	if (!InputTags.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	// GetActivatableAbilities() 返回当前可被激活的技能 Spec,GiveAbility(AbilitySpec);
	for (FGameplayAbilitySpec AbilitySpec : GetActivatableAbilities())
	{
		//查找 AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag); 之前存进的标签 
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTag(InputTags))
		{
			// 标记“输入按下”状态，用于冷却、消耗等预判
			AbilitySpecInputPressed(AbilitySpec);
			// 如果技能没激活，则尝试激活（服务器仲裁）
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle); //释放技能
			}
		}
	}
}

/**
 * 输入系统回调：某输入标签“松开”时调用
 * 同样按标签找到对应技能，标记“输入释放”
 */
void ULrASC::AbilityInputTagReleased(const FGameplayTag& InputTags)
{
	if (!InputTags.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTag(InputTags) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
}

void ULrASC::ApplyDamageToTarget(AActor* Target, const FHitResult& Hit)
{
	UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetASC) return;
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddHitResult(Hit);

	// // todo
	// FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec( DamageGE, 1, Context);
	//
	// if (Spec.IsValid())
	// {
	// 	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	// }
}
