// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/LrGAListDA.h"

const FLrGAConfig* ULrGAListDA::FindGAByTag(const FGameplayTag& GATag) const
{
	for (const FLrGAConfig& GAConfig : GAConfigList)
	{
		if (GAConfig.GATag == GATag)
		{
			return &GAConfig;
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
