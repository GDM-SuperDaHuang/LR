// Fill out your copyright notice in the Description page of Project Settings.


#include "Lib/LrCommonLibrary.h"

#include "AbilitySystemGlobals.h"
#include "Data/LrGAListDA.h"
#include "Game/LrGameModeBase.h"
#include "Kismet/GameplayStatics.h"

UAbilitySystemComponent* ULrCommonLibrary::GetASC(AActor* Actor)
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
}

const FLrDAConfig& ULrCommonLibrary::FindGAByTag(const UObject* WorldContextObject, const FGameplayTag& GATag)
{
	static const FLrDAConfig EmptyConfig; 
	const ALrGameModeBase* LrGameMode = Cast<ALrGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (LrGameMode == nullptr) return EmptyConfig;
	TObjectPtr<ULrGAListDA> LrGAListDA = LrGameMode->LrGAConfigList;
	if (LrGAListDA == nullptr) return EmptyConfig;
	const FLrDAConfig* FindGaByTag = LrGAListDA.Get()->FindGAByTag(GATag);
	if (FindGaByTag == nullptr) return EmptyConfig;
	
	return *FindGaByTag;
}
