// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"


struct FLrGameplayTags
{
public:
	static const FLrGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	/**************************************** 属性 相关 ************************/
	// 玩家主要属性 相关
	FGameplayTag As_HP;
	FGameplayTag As_MaxHP;

	FGameplayTag As_MP;
	FGameplayTag As_MaxMP;

	FGameplayTag As_Endurance; //耐力
	FGameplayTag As_MaxEndurance; //耐力

	
	// 玩家被动属性 相关
	FGameplayTag As_Attack; //攻击
	FGameplayTag As_Defense; //防御

	//攻击类型
	FGameplayTag As_Attack_Type_Magic; //魔法攻击类型
	FGameplayTag As_Attack_Type_Physical; //魔法攻击类型

	//防御类型
	FGameplayTag As_Defense_Type_Magic; //魔法防御类型
	FGameplayTag As_Defense_Type_Physical; //魔法防御类型
	/**************************************** 属性 相关 ************************/


	/**************************************** 输入 相关 ************************/
	FGameplayTag InputTag_Move_WASD;
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_J;
	FGameplayTag InputTag_K;
	FGameplayTag InputTag_L;
	FGameplayTag InputTag_U;
	FGameplayTag InputTag_I;
	FGameplayTag InputTag_O;
	/**************************************** 输入 相关 ************************/


	/**************************************** 技能自身 相关 ************************/
	FGameplayTag GA_1;
	// FGameplayTag GA_2;
	// FGameplayTag GA_3;
	/**************************************** 技能自身 相关 ************************/


	//
	// FGameplayTag Attributes_Secondary_Armor;
	// FGameplayTag Attributes_Secondary_ArmorPenetration;
	// FGameplayTag Attributes_Secondary_BlockChance;
	// FGameplayTag Attributes_Secondary_CriticalHitChance;
	// FGameplayTag Attributes_Secondary_CriticalHitDamage;
	// FGameplayTag Attributes_Secondary_CriticalHitResistance;
	// FGameplayTag Attributes_Secondary_HealthRegeneration;
	// FGameplayTag Attributes_Secondary_ManaRegeneration;
	//
	// FGameplayTag Attributes_Secondary_MaxHealth;
	// FGameplayTag Attributes_Secondary_MaxMana;
	//
	//
	//
	//
	//
	// FGameplayTag Attributes_Meta_IncomingXp;
	//
	//
	//
	// FGameplayTag InputTag_LMB;
	// FGameplayTag InputTag_RMB;
	// FGameplayTag InputTag_1;
	// FGameplayTag InputTag_2;
	// FGameplayTag InputTag_3;
	// FGameplayTag InputTag_4;
	// FGameplayTag InputTag_Passive_1;
	// FGameplayTag InputTag_Passive_2;
	//
	//
	// //蒙太奇事件通知标签
	// FGameplayTag Event_Montage_FireBolt;
	// FGameplayTag Event_Montage_Electrocute;
	// FGameplayTag Event_Montage_ArcaneShards;
	//
	//
	//
	// // FGameplayTag Event_Montage_Attack_Melee;
	// // CombatSocket_Weapon
	// FGameplayTag CombatSocket_Weapon;
	// FGameplayTag CombatSocket_RightHand;
	// FGameplayTag CombatSocket_LeftHand;
	// FGameplayTag CombatSocket_Tail;
	//
	//
	// // Montage tag
	// FGameplayTag Montage_Attack_1;
	// FGameplayTag Montage_Attack_2;
	// FGameplayTag Montage_Attack_3;
	// FGameplayTag Montage_Attack_4;
	//
	//
	// //伤害类型
	// FGameplayTag Damage;
	// FGameplayTag Damage_Fire;
	// FGameplayTag Damage_Lightning;
	// FGameplayTag Damage_Arcane;
	// FGameplayTag Damage_Physical;
	//
	//
	// //伤害抗性
	// FGameplayTag Attributes_Resilience_Fire;
	// FGameplayTag Attributes_Resilience_Lightning;
	// FGameplayTag Attributes_Resilience_Arcane;
	// FGameplayTag Attributes_Resilience_Physical;
	//
	// // debuff
	// FGameplayTag Debuff_Burn;
	// FGameplayTag Debuff_Stun;
	// FGameplayTag Debuff_Arcane;
	// FGameplayTag Debuff_Physical;
	//
	// FGameplayTag Debuff_Chance;
	// FGameplayTag Debuff_Damage;
	// FGameplayTag Debuff_Duration;
	// FGameplayTag Debuff_Frequency;
	//
	//
	// // 被动
	// FGameplayTag Abilities_Passive_HaloOfProtection;
	// FGameplayTag Abilities_Passive_LifeSiphon;
	// FGameplayTag Abilities_Passive_ManaSiphon;
	//
	//
	//
	// FGameplayTag Abilities_None;
	// FGameplayTag Abilities_Attack;
	// FGameplayTag Abilities_Summon;
	//
	//
	//
	// FGameplayTag Abilities_HitReact;
	//
	// FGameplayTag Abilities_Status_Locked;
	// FGameplayTag Abilities_Status_Eligible;
	// FGameplayTag Abilities_Status_Unlocked;
	// FGameplayTag Abilities_Status_Equipped;
	//
	// FGameplayTag Abilities_Type_Offensive;
	// FGameplayTag Abilities_Type_Passive;
	// FGameplayTag Abilities_Type_None;
	//
	//
	//
	// FGameplayTag Abilities_Fire_FireBolt;
	// FGameplayTag Abilities_Fire_FireBlast;	
	// FGameplayTag Abilities_Lightning_Electrocute;
	// FGameplayTag Abilities_Arcane_ArcaneShards;
	//
	//
	//
	// FGameplayTag Cooldown_Fire_FireBolt;
	// FGameplayTag Cooldown_Lighting_Electrocute;
	// FGameplayTag Cooldown_Arcane_ArcaneShards;
	// FGameplayTag Cooldown_Fire_FireBlast;
	//
	//
	//
	//
	// TMap<FGameplayTag, FGameplayTag> DamageTypesToResilience;
	// // TArray<FGameplayTag> DamageTypes;
	// TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs;
	//
	//
	// FGameplayTag Effects_HitReact;
	//

	/**************************************** GameplayCue 相关 ************************/
	FGameplayTag GameplayCue_Melee;
	// FGameplayTag GameplayCue_ShockBurst;
	// FGameplayTag GameplayCue_ShockLoop;
	// FGameplayTag GameplayCue_ArcaneShards;
	// FGameplayTag GameplayCue_FireBlast;
	/**************************************** GameplayCue 相关 ************************/

	
	//
	//        
	//
	// // 输入通知
	// FGameplayTag Play_Block_InputPressed;
	// FGameplayTag Play_Block_InputHeld;
	// FGameplayTag Play_Block_InputReleased;
	// FGameplayTag Play_Block_CursorTrace;

	/**************************************** 蒙太奇通知 相关 ************************/
	FGameplayTag Montage_Event_Attack;
	/**************************************** 蒙太奇通知 相关 ************************/


protected:


private:
	static FLrGameplayTags GameplayTags;
};
