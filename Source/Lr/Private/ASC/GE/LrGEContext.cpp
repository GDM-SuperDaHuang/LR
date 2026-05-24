// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GE/LrGEContext.h"

bool FLrGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint8 RepBits = 0;

	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}

		if (bReplicateEffectCauser && EffectCauser.IsValid())
		{
			RepBits |= 1 << 1;
		}

		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}

		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}

		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}

		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}

		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
	}

	Ar.SerializeBits(&RepBits, 7);

	//-----------------------------------
	// 基础数据同步
	//-----------------------------------

	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}

	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}

	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}

	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}

	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}

	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = MakeShared<FHitResult>();
			}
		}

		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}

	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}

	//-----------------------------------
	// 永远同步 Flags
	//-----------------------------------
	Ar.SerializeBits(&Flags, 8);
	//-----------------------------------
	// 固定局部状态（重点）
	//-----------------------------------
	const bool bHasSpeedCut =
		(Flags & SpeedCut) != 0;

	const bool bHasVertigo =
		(Flags & Vertigo) != 0;

	const bool bHasBurn =
		(Flags & Burn) != 0;

	const bool bHasFrozen =
		(Flags & Frozen) != 0;

	const bool bHasPoison =
		(Flags & Poison) != 0;

	const bool bHasRepel =
		(Flags & Repel) != 0;

	const bool bHasStiffness =
		(Flags & Stiffness) != 0;

	//-----------------------------------
	// Debuff数据
	//-----------------------------------

	if (bHasSpeedCut)
	{
		Ar << SpeedCutRate;
		Ar << SpeedCutDuration;
	}

	if (bHasVertigo)
	{
		Ar << VertigoDuration;
	}

	if (bHasBurn)
	{
		Ar << BurnValue;
		Ar << BurnDuration;
	}

	if (bHasFrozen)
	{
		Ar << FrozenDuration;
	}

	if (bHasPoison)
	{
		Ar << PoisonValue;
		Ar << PoisonDuration;
	}

	if (bHasRepel)
	{
		RepelVector.NetSerialize(Ar, Map, bOutSuccess);
	}

	if (bHasStiffness)
	{
		Ar << StiffnessDuration;
	}

	//-----------------------------------
	// GAS恢复
	//-----------------------------------

	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(),EffectCauser.Get());
	}

	bOutSuccess = true;

	return true;
}
