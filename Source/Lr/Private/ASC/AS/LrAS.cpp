// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/AS/LrAS.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "ASC/GE/LrGEContext.h"
#include "Data/LrBuffDA.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Net/UnrealNetwork.h"
#include "Pawn/LrPawnBase.h"
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

	/* 标签关联绑定  */
	TagsASMaxTags.Add(GameplayTags.As_HP, GameplayTags.As_MaxHP);
	TagsASMaxTags.Add(GameplayTags.As_MP, GameplayTags.As_MaxMP);
	TagsASMaxTags.Add(GameplayTags.As_Endurance, GameplayTags.As_MaxEndurance);
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
		if (GetMaxHP() != 0)
		{
			NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHP());
		}
	}

	if (Attribute == GetMPAttribute())
	{
		if (GetMaxMP() != 0)
		{
			NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMP());
		}
	}

	if (Attribute == GetEnduranceAttribute())
	{
		if (GetMaxEndurance() != 0)
		{
			NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxEndurance());
		}
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
	 * 伤害
	 */
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		float Damage = GetIncomingDamage();
		SetIncomingDamage(0.f);

		const FLrGameplayEffectContext* LrGEContext = static_cast<const FLrGameplayEffectContext*>(Props.EffectContextHandle.Get());
		if (LrGEContext)
		{
			if (LrGEContext->HasFlag(EDamageFlags::Burn))
			{
				ApplyDebuff(*LrGEContext, Props);
			}
		}


		if (Damage > 0 && Props.TargetCharacter->GetClassID() > 0) //只显示敌人特效
		{
			// 构造 Gameplay Cue 参数
			FGameplayCueParameters Params;
			Params.RawMagnitude = Damage; // 将伤害数值存入 Magnitude
			// Params.Location = Data.Target.GetAvatarActor()->GetActorLocation(); // 记录受击位置
			// Params.EffectCauser = Data.EffectSpec.GetEffectContext().GetEffectCauser();
			FLrGameplayTags GameplayTags = FLrGameplayTags::Get();
			// 触发 Gameplay Cue（这会自动在所有客户端同步播放）
			Data.Target.ExecuteGameplayCue(GameplayTags.GameplayCue_Text_Damage, Params);
		}

		float NewHP = GetHP() - Damage;
		if (NewHP <= 0) //死亡
		{
			ILrCombatInterface* CombatInterface = Cast<ILrCombatInterface>(Props.TargetAvatarActor);
			// 击退方向
			FVector KnockDir = (Data.Target.GetAvatarActor()->GetActorLocation() - Props.SourceAvatarActor->GetActorLocation()).GetSafeNormal();
			// 竖直分量
			KnockDir.Z = 0.3f;
			FVector Impulse = KnockDir * 200.f + FVector(0, 0, 200.f); //击飞力*800 + 额外向上力 
			FLrDieParameters LrDieConfig ;
			LrDieConfig.PawnType = Props.TargetCharacter->PawnType;
			LrDieConfig.DeathImpulse = Impulse;
			CombatInterface->ToDie(LrDieConfig);
		}
		/*
		 * HP
		 */
		SetHP(FMath::Clamp(NewHP, 0.f, GetMaxHP()));
	}

	/*
	 * 治疗 
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
	if (LrGEContext.HasFlag(EDamageFlags::Burn))
	{
		EffectClass = LrBuffDA->BurnEffectClass;
	}
	else if (LrGEContext.HasFlag(EDamageFlags::Poison))
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
		LrGEContext.DamageValue);

	// 设置持续时间
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
		SpecHandle,
		GameplayTags.Damage_Base,
		LrGEContext.Duration);

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
	if (IsValid(props.SourceASC) && props.SourceASC->AbilityActorInfo.IsValid() && props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
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
			props.SourceCharacter = Cast<ALrPawnBase>(props.SourceController->GetPawn());
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
		props.TargetCharacter = Cast<ALrPawnBase>(props.TargetAvatarActor);
		// 目标能力系统组件
		props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(props.TargetAvatarActor);
	}
}
