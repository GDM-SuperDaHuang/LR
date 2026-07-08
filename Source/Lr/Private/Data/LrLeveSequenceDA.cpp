// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/LrLeveSequenceDA.h"

FLrLeveSequenceConfig ULrLeveSequenceDA::FindLeveSequenceInfo(const uint16 ApplyPawnType, const uint16 TargetPawnType) const
{
	FLrLeveSequenceConfig LrLeveSequenceConfig;
	for (const FLrCollapseMontageConfig& LrCollapseMontageConfig : LrCollapseMontageConfigs)
	{
		if (LrCollapseMontageConfig.PawnType == ApplyPawnType)
		{
			LrLeveSequenceConfig.ApplyCollapseMontage = LrCollapseMontageConfig.CollapseMontage;
		}
		else if (LrCollapseMontageConfig.PawnType == TargetPawnType)
		{
			LrLeveSequenceConfig.TargetCollapseMontage = LrCollapseMontageConfig.CollapseMontage;
		}
	}
	return LrLeveSequenceConfig;
}
