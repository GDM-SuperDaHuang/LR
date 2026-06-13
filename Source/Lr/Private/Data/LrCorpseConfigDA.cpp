// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/LrCorpseConfigDA.h"

const FLrCorpseConfig* ULrCorpseConfigDA::FindCorpseConfigByPawnType(const uint32 PawnType) const
{
	for (const FLrCorpseConfig& CorpseConfig : LrCorpseConfig)
	{
		if (CorpseConfig.PawnType == PawnType)
		{
			return &CorpseConfig;
		}
	}
	return nullptr;
}
