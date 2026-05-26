// Fill out your copyright notice in the Description page of Project Settings.


#include "Tags/LrGameplayTags.h"

#include "GameplayTagsManager.h"
FLrGameplayTags FLrGameplayTags::GameplayTags;

void FLrGameplayTags::InitializeNativeGameplayTags()
{
	// ==========================================================================================
	// 属性 相关
	// ==========================================================================================
	GameplayTags.As_HP = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("As.HP")),
		FString("HP"));

	GameplayTags.As_MaxHP = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("As.MaxHP")),
		FString("MaxHP"));

	GameplayTags.As_MP = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("As.MP")),
		FString("MP"));

	GameplayTags.As_MaxMP = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("As.MaxMP")),
		FString("MaxMP"));

	GameplayTags.As_Endurance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("As.Endurance")),
		FString("Endurance"));

	GameplayTags.As_MaxEndurance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("As.MaxEndurance")),
		FString("MaxEndurance"));


	GameplayTags.As_Attack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("As.Attack")),
		FString("Attack"));

	GameplayTags.As_Defense = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("As.Defense")),
		FString("Defense"));

	GameplayTags.As_Attack_Type_Magic = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("As.Attack.Type.Magic")),
		FString("Magic Attack"));

	GameplayTags.As_Attack_Type_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("As.Attack.Type.Physical")),
		FString("Physical Attack"));

	GameplayTags.As_Defense_Type_Magic = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("As.Defense.Type.Magic")),
		FString("Magic Defense"));

	GameplayTags.As_Defense_Type_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("As.Defense.Type.Magic")),
		FString("Magic Defense"));
	// ==========================================================================================


	// ==========================================================================================
	// 输入 相关
	// ==========================================================================================
	GameplayTags.InputTag_Move_Forward = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.Move.Forward")),
		FString("Input Move Forward"));
	GameplayTags.InputTag_Move_Backward = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.Move.Backward")),
		FString("Input Move Backward"));
	GameplayTags.InputTag_Move_Left = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.Move.Left")),
		FString("Input Move Left"));
	GameplayTags.InputTag_Move_Right = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.Move.Right")),
		FString("Input Move Right"));

	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.LMB")),
		FString("Input LMB"));

	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.RMB")),
		FString("Input RMB"));

	GameplayTags.InputTag_J = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.J")),
		FString("Input J"));

	GameplayTags.InputTag_K = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.K")),
		FString("Input K"));

	GameplayTags.InputTag_L = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.L")),
		FString("Input L"));

	GameplayTags.InputTag_U = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.U")),
		FString("Input U"));

	GameplayTags.InputTag_I = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.I")),
		FString("Input I"));

	GameplayTags.InputTag_O = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.O")),
		FString("Input O"));
	GameplayTags.InputTag_Jump = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.Jump")),
		FString("InputTag Jump"));

	// ==========================================================================================


	// ==========================================================================================
	// 技能 相关
	// ==========================================================================================
	GameplayTags.GA_Blink = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GA.Blink")),
		FString("GA Blink"));
	
	GameplayTags.GA_Melee_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GA.Melee.1")),
		FString("GA Melee 1"));
	
	GameplayTags.GA_Melee_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GA.Melee.2")),
		FString("GA Melee 2"));
	
	GameplayTags.GA_Melee_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GA.Melee.3")),
		FString("GA Melee 3"));
	
	GameplayTags.GA_6 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GA.6")),
		FString("GA 6"));

	GameplayTags.GA_7 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GA.7")),
		FString("GA 7"));

	GameplayTags.GA_8 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GA.8")),
		FString("GA 8"));

	GameplayTags.GA_9 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GA.9")),
		FString("GA 9"));
	// ==========================================================================================


	// ==========================================================================================
	// 伤害类型 相关
	// ==========================================================================================
	GameplayTags.Damage_Base = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("Damage.Base")),
		FString("Damage Base"));

	// ==========================================================================================


	// ==========================================================================================
	// Start Debuff 相关
	// ==========================================================================================
	GameplayTags.Debuff_SpeedCut = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("Debuff.SpeedCut")),
		FString("Debuff SpeedCut"));

	GameplayTags.Debuff_Vertigo = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("Debuff.Vertigo")),
		FString("Debuff Vertigo"));

	GameplayTags.Debuff_Burn = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("Debuff.Burn")),
		FString("Debuff Burn"));

	GameplayTags.Debuff_Frozen = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("Debuff.Frozen")),
		FString("Debuff Frozen"));

	GameplayTags.Debuff_Poison = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("Debuff.Poison")),
		FString("Debuff Poison"));

	GameplayTags.Debuff_Stiffness = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("Debuff.Stiffness")),
		FString("Debuff Stiffness"));

	// ==========================================================================================


	// ==========================================================================================
	// Start GameplayCue 相关
	// ==========================================================================================
	GameplayTags.GameplayCue_Melee_Trail = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GameplayCue.Melee.Trail")),
		FString("GameplayCue Melee Attack Trail"));

	GameplayTags.GameplayCue_Melee_Hit = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GameplayCue.Melee.Hit")),
		FString("GameplayCue Melee Attack Hit"));

	GameplayTags.GameplayCue_Debuff_SpeedCut = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GameplayCue.Debuff.SpeedCut")),
		FString("GameplayCue Debuff SpeedCut"));

	GameplayTags.GameplayCue_Debuff_Vertigo = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName(TEXT("GameplayCue.Debuff.Vertigo")),
	FString("GameplayCue Debuff Vertigo"));

	GameplayTags.GameplayCue_Debuff_Burn = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName(TEXT("GameplayCue.Debuff.Burn")),
	FString("GameplayCue Debuff Burn"));

	GameplayTags.GameplayCue_Debuff_Frozen = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName(TEXT("GameplayCue.Debuff.Frozen")),
	FString("GameplayCue Debuff Frozen"));

	GameplayTags.GameplayCue_Debuff_Poison = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName(TEXT("GameplayCue.Debuff.Poison")),
	FString("GameplayCue Debuff Poison"));

	GameplayTags.GameplayCue_Debuff_Stiffness = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName(TEXT("GameplayCue.Debuff.Stiffness")),
	FString("GameplayCue Debuff Stiffness"));

	// ==========================================================================================


	// ==========================================================================================
	// 蒙太奇通知 相关
	// ==========================================================================================
	GameplayTags.Montage_Event_Attack_GA1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("Montage.Event.Attack.GA1")),
		FString("Montage Event Attack"));
	GameplayTags.Montage_Event_Attack_GA2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("Montage.Event.Attack.GA2")),
		FString("Montage Event Attack"));
	GameplayTags.Montage_Event_Attack_GA3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("Montage.Event.Attack.GA3")),
		FString("Montage Event Attack"));
	// ==========================================================================================


	// ==========================================================================================
	// 状态 阻塞移动 相关
	// ==========================================================================================
	GameplayTags.State_Block_Move = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("State.Block.Move")),
		FString("Block.Move"));
	// ==========================================================================================
}
