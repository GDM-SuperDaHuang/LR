// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/LrGAListDA.h"

const FLrDAConfig* ULrGAListDA::FindGAByTag(const FGameplayTag& GATag) const
{
	for (const FLrDAConfig& GAConfig : GAConfigList)
	{
		if (GAConfig.GATag == GATag)
		{
			return &GAConfig;
		}
	}

	return nullptr;
}
