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
	FGameplayTag InputTag_Move_Forward; //上
	FGameplayTag InputTag_Move_Backward; //下
	FGameplayTag InputTag_Move_Left; //左
	FGameplayTag InputTag_Move_Right; //右

	FGameplayTag InputTag_Jump;
	FGameplayTag InputTag_Move_WASD;
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_J;
	FGameplayTag InputTag_K;
	FGameplayTag InputTag_L;
	FGameplayTag InputTag_U;
	FGameplayTag InputTag_I;
	FGameplayTag InputTag_O;
	FGameplayTag InputTag_Lightning;


	/**************************************** 输入 相关 ************************/


	/**************************************** 技能自身 相关 ************************/
	FGameplayTag GA_NormalMelee;
	FGameplayTag GA_Blink;
	FGameplayTag GA_Burn;
	FGameplayTag GA_Lightning;


	// 眼魔

	// 飞行触手怪

	/**************************************** 技能自身 相关 ************************/

	/**************************************** 技能冷却 相关 ************************/
	FGameplayTag Data_Cooldown;
	FGameplayTag CD_NormalMelee;
	FGameplayTag CD_Blink;
	FGameplayTag CD_Burn;
	FGameplayTag CD_Lightning;

	/**************************************** 技能冷却 相关 ************************/


	//
	/**************************************** 伤害类型 相关 ************************/
	FGameplayTag Damage_Base;
	// FGameplayTag Damage_Fire;
	// FGameplayTag Damage_Lightning;
	// FGameplayTag Damage_Arcane;
	// FGameplayTag Damage_Physical;
	/**************************************** 伤害类型 相关 ************************/

	// TMap<FGameplayTag, FGameplayTag> DamageTypesToResilience;
	// // TArray<FGameplayTag> DamageTypes;
	// TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs;
	//


	/**************************************** Debuff 相关 ************************/
	FGameplayTag Debuff_SpeedCut;
	FGameplayTag Debuff_Vertigo;
	FGameplayTag Debuff_Burn;
	FGameplayTag Debuff_Frozen;
	FGameplayTag Debuff_Poison;
	FGameplayTag Debuff_Stiffness;
	/**************************************** Debuff 相关 ************************/

	/**************************************** GameplayCue 相关 ************************/


	FGameplayTag GameplayCue_Melee_Trail; //轨迹特效
	FGameplayTag GameplayCue_Melee_Hit; // 命中特效

	FGameplayTag GameplayCue_Text_Damage; //文本伤害

	FGameplayTag GameplayCue_Debuff_SpeedCut;
	FGameplayTag GameplayCue_Debuff_Vertigo;
	FGameplayTag GameplayCue_Debuff_Burn;
	FGameplayTag GameplayCue_Debuff_Frozen;
	FGameplayTag GameplayCue_Debuff_Poison;
	FGameplayTag GameplayCue_Debuff_Stiffness;

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

	/**************************************** 状态 阻塞移动 相关 ************************/
	FGameplayTag State_Block_Move;
	/**************************************** 状态 阻塞移动 相关 ************************/


protected:


private:
	static FLrGameplayTags GameplayTags;
};
