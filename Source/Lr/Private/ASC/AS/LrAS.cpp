// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/AS/LrAS.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "ASC/GE/LrGEContext.h"
#include "Data/LrBuffDA.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Net/UnrealNetwork.h"
#include "Tags/LrGameplayTags.h"

ULrAS::ULrAS()
{
	const FLrGameplayTags GameplayTags = FLrGameplayTags::Get();

	/* 标签绑定属性  */
	/** primary */
	// FAttributeSignature StrengthDelegate;
	// StrengthDelegate.BindStatic(HP);
	TagsASMap.Add(GameplayTags.As_HP, GetHPAttribute);
	TagsASMap.Add(GameplayTags.As_MaxHP, GetMaxHPAttribute);
	TagsASMap.Add(GameplayTags.As_MP, GetMPAttribute);
	TagsASMap.Add(GameplayTags.As_MaxMP, GetMaxMPAttribute);
	TagsASMap.Add(GameplayTags.As_Endurance, GetEnduranceAttribute);
	TagsASMap.Add(GameplayTags.As_MaxEndurance, GetMaxEnduranceAttribute);


	// TagsToAttributes.Add(GameplayTags.Attributes_Primary_Intelligence, GetIntelligenceAttribute);
	// TagsToAttributes.Add(GameplayTags.Attributes_Primary_Resilience, GetResilienceAttribute);
	// TagsToAttributes.Add(GameplayTags.Attributes_Primary_Vigor, GetVigorAttribute);
}

void ULrAS::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	/**
	 * DOREPLIFETIME_CONDITION_NOTIFY 参数,GAS专用宏
	 * 1.当前类名
	 * 2.属性名
	 * 3.复制条件（COND_None/COND_OwnerOnly/COND_SkipOwner 等）
	 * 
	 * 4.通知策略（REPNOTIFY_Always/REPNOTIFY_OnChanged）
	 * 		– REPNOTIFY_OnChanged（默认值改变才触发）
	 * 		– REPNOTIFY_Always（只要收到包就触发，用于预测回滚）
	 */
	DOREPLIFETIME_CONDITION_NOTIFY(ULrAS, HP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULrAS, MaxHP, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(ULrAS, MP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULrAS, MaxMP, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(ULrAS, Endurance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULrAS, MaxEndurance, COND_None, REPNOTIFY_Always);
}

/**
 * 属性即将被修改
 * 执行时机是属性值即将被修改但尚未实际更新到 FGameplayAttributeData 之前
 * NewValue : 即将写入 BaseValue 或 CurrentValue 前的目标值。
 */
void ULrAS::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetHPAttribute())
	{
		// 限制Health在0到MaxHealth之间
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHP());
		// UE_LOG(LogTemp, Warning, TEXT("Health=%f"), NewValue);
	}

	if (Attribute == GetMPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMP());
	}

	if (Attribute == GetEnduranceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxEndurance());
	}
}

/**
 * 
 * 伤害结算 / 生命扣减 / 副作用
 * 
 * GE 执行完成后（BaseValue 已落地）
 * PostGameplayEffectExecute 可有可无，这里修改的是连带属性，
 * 比如有一个施加中毒GE，需要ui，特效等显示，这个不适合直接修改Hp，需要中间属性Damage的属性进行了变化，这时可以对HP属性修改。
 * 
 */
/* ---------- GE 执行完毕，本次 GE 对属性的数值修改已经“落袋”的回调 ---------- */
void ULrAS::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props); // 解析效果的源和目标信息
	/*
	 * Damage
	 */
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		float Damage = GetIncomingDamage();
		SetIncomingDamage(0.f);

		/*
		 * Shield First
		 */
		float CurrentShield = GetShield();
		if (CurrentShield > 0.f)
		{
			const float NewShield = FMath::Max(0.f, CurrentShield - Damage);
			float RemainDamage = FMath::Max(0.f, Damage - CurrentShield);
			SetShield(NewShield);
			Damage = RemainDamage;
		}

		/*
		 * Armor Reduce
		 */
		Damage -= GetDefense();
		Damage = FMath::Max(0.f, Damage);

		const FLrGameplayEffectContext* LrGEContext = static_cast<const FLrGameplayEffectContext*>(Props.EffectContextHandle.Get());
		if (LrGEContext)
		{
			if (LrGEContext->HasFlag(Burn))
			{
				ApplyDebuff(*LrGEContext, Props);
			}
		}


		/*
		 * HP
		 */
		SetHP(FMath::Clamp(GetHP() - Damage, 0.f, GetMaxHP()));
	}

	/*
	 * Heal
	 */
	if (Data.EvaluatedData.Attribute == GetIncomingHealAttribute())
	{
		float Heal = GetIncomingHeal();
		SetIncomingHeal(0.f);
		SetHP(FMath::Clamp(GetHP() + Heal, 0.f, GetMaxHP()));
	}
}

/**
 * GE 执行完毕，本次 GE 对属性的数值修改已经“落袋”的回调
 * UI / 状态监听 / 触发事件 /动画 / 特效触发
 * 禁止修改否则，无限递归
 */
void ULrAS::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}


/************* 回 调 相 关*****************/
void ULrAS::OnRepHp(const FGameplayAttributeData& OldValue) const
{
	/******************************************
 	* 属性复制回调函数
 	* 当属性通过网络复制时调用
 	* 主要用于处理客户端接收到新属性值后的逻辑
 	* GAMEPLAYATTRIBUTE_REPNOTIFY 为专供 FGameplayAttributeData 的 OnRep 实现宏；
 	* GAS 专用” 的 OnRep 实现，负责 值同步 + 预测回滚
 	******************************************/
	/**
	 * 把 OldHealth 登记到 ASC 的 PredictionRollback 表
	 * 把 当前值 更新为 网络新值（ASC 内部缓存）
	 */
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULrAS, HP, OldValue);
}

void ULrAS::OnRepMp(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULrAS, MP, OldValue);
}

void ULrAS::OnRepEndurance(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULrAS, Endurance, OldValue);
}

void ULrAS::ApplyDebuff(const FLrGameplayEffectContext& LrGEContext, const FEffectProperties& Props)
{
	// FLrGameplayTags GameplayTags = FLrGameplayTags::Get();
	// FGameplayEffectContextHandle EffectContext = Props.SourceASC->MakeEffectContext();
	// EffectContext.AddSourceObject(Props.SourceAvatarActor);
	// // todo 
	// UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), FName("DebuffName"));
	// // 3. 配置GE属性
	// Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration; // 持续型
	// Effect->Period = 1; // 周期触发
	// Effect->DurationMagnitude = FScalableFloat(LrGEContext.BurnDuration); // 总时长
	// // 4. 【关键】添加Modifier，这才是周期伤害的来源！
	// // 使用SetByCallerMagnitude，这样每次周期都能动态获取伤害值
	// FInheritedTagContainer TagContainer = FInheritedTagContainer();
	// UTargetTagsGameplayEffectComponent& Component = Effect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
	// TagContainer.Added.AddTag(GameplayTags.Debuff_Burn);
	// Component.SetAndApplyTargetTagChanges(TagContainer);
	// Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Debuff_Burn);
	//
	// // 5. 配置堆叠规则
	// Effect->StackingType = EGameplayEffectStackingType::AggregateBySource;
	// Effect->StackLimitCount = 1;//同一来源只能存在一个实例，刷新持续时间而非叠加多层。
	// int32 Index = Effect->Modifiers.Num();
	// Effect->Modifiers.Add(FGameplayModifierInfo());
	// FGameplayModifierInfo& ModifierInfo = Effect->Modifiers[Index];
	// // 6. 【核心】添加伤害Modifier
	// ModifierInfo.ModifierMagnitude = FScalableFloat(LrGEContext.BurnValue); // 伤害值
	// ModifierInfo.ModifierOp = EGameplayModOp::Additive; // 累加操作
	// ModifierInfo.Attribute = ULrAS::GetIncomingDamageAttribute(); // 目标属性
	//
	// // 7. 【最终应用】将GE应用到目标！
	// if (FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(Effect, EffectContext, 1.f))
	// {
	// 	// 8. 真正应用周期伤害GE（服务器执行）
	// 	FActiveGameplayEffectHandle ApplyGameplayEffectSpecToSelf = Props.TargetASC->ApplyGameplayEffectSpecToSelf(*MutableSpec);
	// }


	if (!Props.SourceASC || !Props.TargetASC)
	{
		return;
	}

	FLrGameplayTags GameplayTags = FLrGameplayTags::Get();

	// 创建 Context
	FGameplayEffectContextHandle EffectContext = Props.SourceASC->MakeEffectContext();

	// 设置来源对象
	EffectContext.AddSourceObject(Props.SourceAvatarActor);

	// 最终要应用的 GE
	TSubclassOf<UGameplayEffect> EffectClass = nullptr;

	// 根据 Debuff 类型选择 GE
	if (LrGEContext.HasFlag(Burn))
	{
		EffectClass = LrBuffDA->BurnEffectClass;
	}
	else if (LrGEContext.HasFlag(Poison))
	{
		EffectClass = LrBuffDA->PoisonEffectClass;
	}

	if (!EffectClass)
	{
		return;
	}

	// 创建 Spec
	FGameplayEffectSpecHandle SpecHandle = Props.SourceASC->MakeOutgoingSpec(
		EffectClass,
		1.f,
		EffectContext);

	if (!SpecHandle.IsValid())
	{
		return;
	}

	// 设置 DOT 伤害
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
		SpecHandle,
		GameplayTags.Damage_Base,
		LrGEContext.BurnValue);

	// 设置持续时间
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
		SpecHandle,
		GameplayTags.Damage_Base,
		LrGEContext.BurnDuration);

	// Apply 到目标
	Props.TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}


/* ---------- 工具：提取上下文 ---------- */
void ULrAS::SetEffectProperties(const struct FGameplayEffectModCallbackData& Data, FEffectProperties& props) const
{
	// 1. 获取效果上下文
	props.EffectContextHandle = Data.EffectSpec.GetContext();
	// 2. 获取源能力系统组件（施加效果的一方）
	props.SourceASC = props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	// 3. 获取源相关信息
	if (IsValid(props.SourceASC) && props.SourceASC->AbilityActorInfo.IsValid() && props.SourceASC->AbilityActorInfo->
	                                                                                     AvatarActor.IsValid())
	{
		// 源Avatar角色（通常是角色实例）
		props.SourceAvatarActor = props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		// 源控制器
		props.SourceController = props.SourceASC->AbilityActorInfo->PlayerController.Get();
		// 如果直接获取控制器失败，尝试通过Pawn获取
		if (props.SourceController == nullptr && props.SourceAvatarActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(props.SourceAvatarActor))
			{
				props.SourceController = Pawn->GetController();
			}
		}
		// 源角色
		if (props.SourceController != nullptr)
		{
			props.SourceCharacter = Cast<APawn>(props.SourceController->GetPawn());
		}
	}

	// 4. 获取目标相关信息（接收效果的一方）
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		// 目标Avatar角色
		props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		// 目标控制器
		props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		// 目标角色
		props.TargetCharacter = Cast<APawn>(props.TargetAvatarActor);
		// 目标能力系统组件
		props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(props.TargetAvatarActor);
	}
}
