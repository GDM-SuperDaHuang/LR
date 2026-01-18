// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/LrASC.h"

#include "ASC/GA/LrGABase.h"

void ULrASC::AddGA(const TArray<TSubclassOf<UGameplayAbility>>& GAList)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : GAList)
	{
		// 创建技能规格（Spec），Level 1，默认不给予输入
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		// 转成咱们自己的基类，取出编辑器里填的 StartupInputTag
		if (const ULrGABase* LrGaBase = Cast<ULrGABase>(AbilitySpec.Ability))
		{
			// 把输入标签塞进 DynamicAbilityTags，后续输入系统按标签检索
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(LrGaBase->InputTag);
			// AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
			
			/**
			 * GiveAbility
			 * 1. 生成全局唯一 Handle （通常一种技能一个）
			 * 2. 实例化 GA 对象（CDO → 实例），里面初始化了AvatarActor ，OwnerActor，PlayerController 
			 * 3. 塞进 ActiveAbilitySpecs 数组
			 * 4. 标记网络脏，客户端会收到复制包并本地生成影子 Spec
			 */
			GiveAbility(AbilitySpec); // 真正交给 ASC 管理
		}

		//  todo 技能初始化广播 ？？
		// bStartUpAbilitiesGiven = true;
		// AbilitiesGivenDelegate.Broadcast();
	}
}
