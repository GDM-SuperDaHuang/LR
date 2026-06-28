// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/LrASC.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "ASC/GA/LrGABase.h"
#include "ASC/GE/LrGEContext.h"
#include "Data/LrBuffDA.h"
#include "Data/LrGAListDA.h"
#include "Lib/LrCommonLibrary.h"
#include "Lr/Lr.h"
#include "Pawn/LrPawnBase.h"
#include "Tags/LrGameplayTags.h"

// void ULrASC::AddGA(const TArray<FGameplayTag>& GATagList)
// {
// 	for (const FGameplayTag GATag : GATagList)
// 	{
// 		FLrGAConfig LrDAConfig = ULrCommonLibrary::FindGAByTag(GetOwner(), GATag);
// 		// 创建技能规格（Spec），Level 1，默认不给予输入
// 		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(LrDAConfig.GAClass, 1);
// 		// 输入绑定
// 		AbilitySpec.GetDynamicSpecSourceTags().AddTag(LrDAConfig.InputTag); //InputTag
// 		/**
// 		* GiveAbility
// 		* 1. 生成全局唯一 Handle （通常一种技能一个）
// 		* 2. 实例化 GA 对象（CDO → 实例），里面初始化了AvatarActor ，OwnerActor，PlayerController 
// 		* 3. 塞进 ActiveAbAbilityInputTagPressedilitySpecs 数组
// 		* 4. 标记网络脏，客户端会收到复制包并本地生成影子 Spec
// 		*/
// 		GiveAbility(AbilitySpec); // 真正交给 ASC 管理
// 		UE_LOG(LogTemp, Warning, TEXT("Client Ability Count: %d"), GetActivatableAbilities().Num());
// 		//  todo 技能初始化广播 ？？
// 		// bStartUpAbilitiesGiven = true;
// 		// AbilitiesGivenDelegate.Broadcast();
// 	}
// }

void ULrASC::AddAllGA(ALrPawnBase* PawnBase)
{
	if (PawnBase)
	{
		ALrGameModeBase* LrGameModeBase = ULrCommonLibrary::GetLrGameModeBase(GetOwner());
		if (LrGameModeBase)
		{
			ULrGAListDA* LrGaListDa = LrGameModeBase->LrGAListDA;
			if (LrGaListDa)
			{
				for (FLrPawnTypeGAConfig LrAllPawnTypeGaConfig : LrGaListDa->LrAllPawnTypeGAConfig)
				{
					if (LrAllPawnTypeGaConfig.PawnType == PawnBase->PawnType)
					{
						for (FLrGAConfig LrDAConfig : LrAllPawnTypeGaConfig.AllLrGAConfig)
						{
							// 创建技能规格（Spec），Level 1，默认不给予输入
							FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(LrDAConfig.GAClass, 1);
							// 输入绑定
							// AbilitySpec.GetDynamicSpecSourceTags().AddTag(LrDAConfig.InputTag); //InputTag
							AbilitySpec.InputID = LrDAConfig.InputId;
							GiveAbility(FGameplayAbilitySpec(LrDAConfig.GAClass, 1, LrDAConfig.InputId));// 真正交给 ASC 管理
							// GiveAbility(AbilitySpec); // 真正交给 ASC 管理
						}
						return;
					}
				}
			}
		}
	}
}

// void ULrASC::AbilityInputTagPressed(const FGameplayTag& InputTags)
// {
// 	if (!InputTags.IsValid()) return;
// 	FScopedAbilityListLock ActiveScopeLoc(*this);
// 	TArray<FGameplayAbilitySpec> GameplayAbilitySpecs = GetActivatableAbilities();
// 	for (FGameplayAbilitySpec AbilitySpec : GameplayAbilitySpecs)
// 	{
// 		// 匹配输入
// 		if (AbilitySpec.GetDynamicSpecSourceTags().HasTag(InputTags)) //这里可以优化为一个映射层
// 		{
// 			AbilitySpecInputPressed(AbilitySpec);
// 			if (!AbilitySpec.IsActive())
// 			{
// 				TryActivateAbility(AbilitySpec.Handle); //释放技能
// 			}
// 		}
// 	}
// }
//
// /**
//  * 输入系统回调：某输入标签被“按住”时调用
//  * 遍历所有已拥有的技能，找到 DynamicAbilityTags 匹配的标签 → 尝试激活
//  */
// void ULrASC::AbilityInputTagHeld(const FGameplayTag& InputTags)
// {
// 	if (!InputTags.IsValid()) return;
// 	FScopedAbilityListLock ActiveScopeLoc(*this);
// 	// GetActivatableAbilities() 返回当前可被激活的技能 Spec,GiveAbility(AbilitySpec);
// 	for (FGameplayAbilitySpec AbilitySpec : GetActivatableAbilities())
// 	{
// 		//查找 AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag); 之前存进的标签 
// 		if (AbilitySpec.GetDynamicSpecSourceTags().HasTag(InputTags))
// 		{
// 			// 标记“输入按下”状态，用于冷却、消耗等预判,把 AbilitySpec 里的 InputPressed 标志设为 true,Ability 内部可以查询：IsInputPressed()
// 			AbilitySpecInputPressed(AbilitySpec);
// 			if (AbilitySpec.IsActive())
// 			{
// 				if (UGameplayAbility* AbilityInstance = AbilitySpec.GetPrimaryInstance())
// 				{
// 					InvokeReplicatedEvent(
// 						EAbilityGenericReplicatedEvent::InputPressed,
// 						AbilitySpec.Handle,
// 						AbilityInstance->GetCurrentActivationInfo().GetActivationPredictionKey()
// 					);
// 				}
// 			}
// 		}
// 	}
// }
//
// /**
//  * 输入系统回调：某输入标签“松开”时调用
//  * 同样按标签找到对应技能，标记“输入释放”
//  */
// void ULrASC::AbilityInputTagReleased(const FGameplayTag& InputTags)
// {
// 	if (!InputTags.IsValid()) return;
// 	FScopedAbilityListLock ActiveScopeLoc(*this);
// 	for (FGameplayAbilitySpec AbilitySpec : GetActivatableAbilities())
// 	{
// 		if (AbilitySpec.GetDynamicSpecSourceTags().HasTag(InputTags) && AbilitySpec.IsActive())
// 		{
// 			AbilitySpecInputReleased(AbilitySpec);
// 			if (UGameplayAbility* AbilityInstance = AbilitySpec.GetPrimaryInstance())
// 			{
// 				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilityInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
// 			}
// 		}
// 	}
// }

void ULrASC::AbilityInputTagPressed0(const int32 InputId)
{
	//防止你在遍历 Ability 列表时，它被同时修改（比如添加/移除 Ability）
	FScopedAbilityListLock ActiveScopeLoc(*this);
	TArray<FGameplayAbilitySpec> GameplayAbilitySpecs = GetActivatableAbilities();
	for (FGameplayAbilitySpec AbilitySpec : GameplayAbilitySpecs)
	{
		if (AbilitySpec.InputID == InputId)
		{
			if (InputId < LrInputID::Hold) //短按技能
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
			else //长按技能
			{
				// 标记按下
				// AbilitySpecInputPressed(AbilitySpec);
				// GAS 输入链路，WaitInputRelease 依赖它
				AbilityLocalInputPressed(InputId);
			}
		}
	}
}

void ULrASC::AbilityInputTagHeld0(const int32& InputId)
{
	// ✔ 仍然走标准输入
	FScopedAbilityListLock ActiveScopeLoc(*this);
	// GetActivatableAbilities() 返回当前可被激活的技能 Spec,GiveAbility(AbilitySpec);
	for (FGameplayAbilitySpec AbilitySpec : GetActivatableAbilities())
	{
		//查找 AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag); 之前存进的标签 
		// ✔ 仍然走标准输入
		if (AbilitySpec.InputID == InputId)
		{
			// 已激活就什么也不用做
			if (AbilitySpec.IsActive())
			{
				continue;
			}
			if (InputId < LrInputID::Hold) //短按技能
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
			// AbilityLocalInputPressed(InputId);
		}
	}
}

void ULrASC::AbilityInputTagReleased0(const int32& InputId)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.InputID == InputId)
		{
			if (InputId < LrInputID::Hold)
			{
				continue;
			}
			// 标记释放
			// AbilitySpecInputReleased(AbilitySpec);
			// GAS 输入释放链路，WaitInputRelease 必须
			AbilityLocalInputReleased(InputId);
		}
	}
}


void ULrASC::ApplyDamageToTarget(AActor* Target, FDamageEffectParams DamageEffectParams) const
{
	UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetASC || !SourceASC) return;
	const FLrGameplayTags GameplayTags = FLrGameplayTags::Get();

	// 1. 【创建基础上下文】—— GAS效果的"身份证",
	FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
	FLrGameplayEffectContext* LrContext = static_cast<FLrGameplayEffectContext*>(EffectContextHandle.Get());
	// Context.AddHitResult(Hit);

	// 记录伤害来源（用于仇恨、击杀统计）
	EffectContextHandle.AddSourceObject(Target);

	// 2. 【扩展上下文数据】—— 这是核心创新点！
	// 将物理反馈参数存入自定义上下文，供ExecutionCalculation读取
	LrContext->Flags = DamageEffectParams.Flags;
	LrContext->SpeedCutRate = DamageEffectParams.SpeedCutRate;
	LrContext->Duration = DamageEffectParams.Duration;
	LrContext->DamageValue = DamageEffectParams.DamageValue;
	LrContext->RepelVector = DamageEffectParams.RepelVector;

	// todo 4. 【创建效果规格】—— GE_Damage的"执行计划书" 
	TSubclassOf<UGameplayEffect> LrGEClass;
	const bool bHasSpeedCut = LrContext->HasFlag(LrContext->Flags, EDamageFlags::SpeedCut);
	const bool bHasVertigo = LrContext->HasFlag(LrContext->Flags, EDamageFlags::Vertigo);
	const bool bHasBurn = LrContext->HasFlag(LrContext->Flags, EDamageFlags::Burn);
	const bool bHasFrozen = LrContext->HasFlag(LrContext->Flags, EDamageFlags::Frozen);
	const bool bHasPoison = LrContext->HasFlag(LrContext->Flags, EDamageFlags::Poison);
	const bool bHasRepel = LrContext->HasFlag(LrContext->Flags, EDamageFlags::Repel);
	const bool bHasStiffness = LrContext->HasFlag(LrContext->Flags, EDamageFlags::Stiffness);

	const ULrBuffDA* LrBuffDA = ULrCommonLibrary::GetGEDA(this);
	if (bHasSpeedCut)
	{
		LrGEClass = LrBuffDA->MeleeAttackEffectClass;
	}
	else if (bHasVertigo)
	{
		LrGEClass = LrBuffDA->MeleeAttackEffectClass;
	}
	else if (bHasBurn)
	{
		LrGEClass = LrBuffDA->BurnEffectClass;
	}
	else if (bHasFrozen)
	{
		LrGEClass = LrBuffDA->MeleeAttackEffectClass;
	}
	else if (bHasPoison)
	{
		LrGEClass = LrBuffDA->PoisonEffectClass;
	}
	else if (bHasRepel)
	{
		LrGEClass = LrBuffDA->MeleeAttackEffectClass;
	}
	else if (bHasStiffness)
	{
		LrGEClass = LrBuffDA->MeleeAttackEffectClass;
	}
	else
	{
		LrGEClass = LrBuffDA->DefaultEffectClass;
	}

	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(LrGEClass, 1, EffectContextHandle);
	//设置时长
	if (bHasBurn || bHasPoison)
	{
		SpecHandle.Data->SetDuration(LrContext->Duration, true);
	}

	// 有问题？
	// 5. 【SetByCaller动态赋值】—— 运行时注入伤害值，避免硬编码
	// 使用SetByCallerMagnitude可在ExecCalc中通过 Spec.GetSetByCallerMagnitude()读取
	UAbilitySystemBlueprintLibrary::AssignSetByCallerMagnitude(SpecHandle, LrGEKeys::Flags, static_cast<float>(DamageEffectParams.Flags));
	UAbilitySystemBlueprintLibrary::AssignSetByCallerMagnitude(SpecHandle, LrGEKeys::SpeedCutRate, DamageEffectParams.SpeedCutRate);
	UAbilitySystemBlueprintLibrary::AssignSetByCallerMagnitude(SpecHandle, LrGEKeys::Duration, DamageEffectParams.Duration);
	UAbilitySystemBlueprintLibrary::AssignSetByCallerMagnitude(SpecHandle, LrGEKeys::DamageValue, DamageEffectParams.DamageValue);

	// 7. 【应用效果到目标】—— 最终触发ExecCalc_Damage::Execute_Implementation(),需要蓝图中配置ExecCalc_Damage计算类
	// 注意：这里无论是否预测都会执行，但客户端的预测版本会被服务器校正
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}
