// Fill out your copyright notice in the Description page of Project Settings.


#include "Lib/LrCommonLibrary.h"

#include "Data/LrGAListDA.h"
#include "Game/LrGameModeBase.h"
#include "Kismet/GameplayStatics.h"


const FLrDAConfig* ULrCommonLibrary::FindGAByTag(const UObject* WorldContextObject,const FGameplayTag& GATag)
{
	const ALrGameModeBase* LrGameMode = Cast<ALrGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (LrGameMode == nullptr) return nullptr;
	TObjectPtr<ULrGAListDA> LrGAListDA = LrGameMode->LrGAConfigList;
	if (LrGAListDA == nullptr) return nullptr;
	return LrGAListDA.Get()->FindGAByTag(GATag);
}
