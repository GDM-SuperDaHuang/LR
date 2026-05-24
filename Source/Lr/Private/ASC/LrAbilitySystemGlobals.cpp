// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/LrAbilitySystemGlobals.h"

#include "ASC/GE/LrGEContext.h"

FGameplayEffectContext* ULrAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FLrGameplayEffectContext();
}
