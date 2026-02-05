// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/AS/LrAS.h"

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
