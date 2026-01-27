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
	GameplayTags.InputTag_Move_WASD = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("InputTag.Move.WASD")),
		FString("Input Move WASD"));

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
	// ==========================================================================================


	// ==========================================================================================
	// 技能 相关
	// ==========================================================================================
	GameplayTags.GA_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GA.1")),
		FString("GA 1"));

	GameplayTags.GA_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GA.2")),
		FString("GA 2"));

	GameplayTags.GA_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(TEXT("GA.3")),
		FString("GA 3"));
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
