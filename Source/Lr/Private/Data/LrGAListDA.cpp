// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/LrGAListDA.h"

// const FLrGAConfig* ULrGAListDA::FindGAByTag(const FGameplayTag& GATag) const
// {
// 	for (const FLrGAConfig& GAConfig : GAConfigList)
// 	{
// 		if (GAConfig.GATag == GATag)
// 		{
// 			return &GAConfig;
// 		}
// 	}
//
// 	return nullptr;
// }

const FLrGAConfig* ULrGAListDA::FindGA(const uint16 PawnType, const FGameplayTag& GATag) const
{
	for (const FLrPawnTypeGAConfig& Config : LrAllPawnTypeGAConfig)
	{
		if (Config.PawnType != PawnType )
		{
			continue;
		}
		for (const FLrGAConfig& AllLrGaConfig : Config.AllLrGAConfig)
		{
			if (AllLrGaConfig.GATag == GATag)
			{
				return &AllLrGaConfig;
			}
		}
	}
	return nullptr;
}

const FLrNSConfig* ULrGAListDA::FindNSByTag(const FGameplayTag& Tag) const
{
	for (const FLrNSConfig& Config : NSConfigList)
	{
		if (Config.NSTag == Tag)
		{
			return &Config;
		}
	}
	return nullptr;
}

const FLrWeaponConfig* ULrGAListDA::FindWeaponByID(const int32 WeaponID) const
{
	for (const FLrWeaponConfig& Config : LrWeaponConfigList)
	{
		if (Config.WeaponID == WeaponID)
		{
			return &Config;
		}
	}
	return nullptr;
}

// const FPawnTypeGAConfig* ULrGAListDA::FindPawnTypeGA(const uint16 PawnType) const
// {
// 	for (const FPawnTypeGAConfig& PawnTypeGAConfig : PawnTypeGAConfigList)
// 	{
// 		if (PawnTypeGAConfig.PawnType == PawnType)
// 		{
// 			return &PawnTypeGAConfig;
// 		}
// 	}
// 	return nullptr;
// }

