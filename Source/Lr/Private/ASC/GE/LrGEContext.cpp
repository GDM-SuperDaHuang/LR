// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GE/LrGEContext.h"

bool FLrGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	return FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);
}
