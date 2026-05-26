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
	const bool bHasSpeedCut = EnumHasAnyFlags(Flags, EDamageFlags::SpeedCut);
	const bool bHasVertigo = EnumHasAnyFlags(Flags, EDamageFlags::Vertigo);
	const bool bHasBurn = EnumHasAnyFlags(Flags, EDamageFlags::Burn);
	const bool bHasFrozen = EnumHasAnyFlags(Flags, EDamageFlags::Frozen);
	const bool bHasPoison = EnumHasAnyFlags(Flags, EDamageFlags::Poison);
	const bool bHasRepel = EnumHasAnyFlags(Flags, EDamageFlags::Repel);
	const bool bHasStiffness = EnumHasAnyFlags(Flags, EDamageFlags::Stiffness);
	//-----------------------------------
	// Debuff数据
	//-----------------------------------

	if (bHasSpeedCut)
	{
		Ar << SpeedCutRate;
		Ar << Duration;
	}

	if (bHasVertigo)
	{
		Ar << Duration;
	}

	if (bHasBurn)
	{
		Ar << DamageValue;
		Ar << Duration;
	}

	if (bHasFrozen)
	{
		Ar << Duration;
	}

	if (bHasPoison)
	{
		Ar << DamageValue;
		Ar << Duration;
	}

	if (bHasRepel)
	{
		RepelVector.NetSerialize(Ar, Map, bOutSuccess);
	}

	if (bHasStiffness)
	{
		Ar << Duration;
	}

	//-----------------------------------
	// GAS恢复
	//-----------------------------------

	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get());
	}

	bOutSuccess = true;

	return true;
}
