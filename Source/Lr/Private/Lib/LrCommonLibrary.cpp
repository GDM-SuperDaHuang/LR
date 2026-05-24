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

ALrGameModeBase* ULrCommonLibrary::GetLrGameModeBase(const UObject* WorldContextObject)
{
	return Cast<ALrGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
}

const FLrGAConfig& ULrCommonLibrary::FindGAByTag(const UObject* WorldContextObject, const FGameplayTag& GATag)
{
	static const FLrGAConfig EmptyConfig;
	const ALrGameModeBase* LrGameMode = Cast<ALrGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (LrGameMode == nullptr) return EmptyConfig;
	TObjectPtr<ULrGAListDA> LrGAListDA = LrGameMode->LrGAListDA;
	if (LrGAListDA == nullptr) return EmptyConfig;
	const FLrGAConfig* FindGaByTag = LrGAListDA.Get()->FindGAByTag(GATag);
	if (FindGaByTag == nullptr) return EmptyConfig;
	return *FindGaByTag;
}

const FPawnTypeGAConfig& ULrCommonLibrary::FindPawnTypeGAConfig(const UObject* WorldContextObject, const uint16 PawnType)
{
	static const FPawnTypeGAConfig EmptyConfig;
	const ALrGameModeBase* LrGameMode = Cast<ALrGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (LrGameMode == nullptr) return EmptyConfig;
	TObjectPtr<ULrGAListDA> LrGAListDA = LrGameMode->LrGAListDA;
	if (LrGAListDA == nullptr) return EmptyConfig;
	const FPawnTypeGAConfig* FindGaByTag = LrGAListDA.Get()->FindPawnTypeGA(PawnType);
	if (FindGaByTag == nullptr) return EmptyConfig;
	return *FindGaByTag;
}


FLrNSConfig ULrCommonLibrary::FindNSByTag(const UObject* WorldContextObject, const FGameplayTag& GATag)
{
	static const FLrNSConfig EmptyConfig; // 只读兜底
	const ALrGameModeBase* LrGameMode = Cast<ALrGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (LrGameMode == nullptr) return EmptyConfig;
	TObjectPtr<ULrGAListDA> LrGAListDA = LrGameMode->LrGAListDA;
	if (LrGAListDA == nullptr) return EmptyConfig;
	const FLrNSConfig* Config = LrGAListDA.Get()->FindNSByTag(GATag);
	if (Config == nullptr) return EmptyConfig;
	return *Config;
}

FLrWeaponConfig ULrCommonLibrary::FindWeaponByID(const UObject* WorldContextObject, const int32 WeaponID)
{
	static const FLrWeaponConfig EmptyConfig; // 只读兜底
	const ALrGameModeBase* LrGameMode = Cast<ALrGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (LrGameMode == nullptr) return EmptyConfig;
	TObjectPtr<ULrGAListDA> LrGAListDA = LrGameMode->LrGAListDA;
	if (LrGAListDA == nullptr) return EmptyConfig;
	const FLrWeaponConfig* Config = LrGAListDA.Get()->FindWeaponByID(WeaponID);
	if (Config == nullptr) return EmptyConfig;
	return *Config;
}

const ULrBuffDA* ULrCommonLibrary::GetGEDA(const UObject* WorldContextObject)
{
	const ALrGameModeBase* LrGameMode = Cast<ALrGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (LrGameMode == nullptr) return nullptr;
	return LrGameMode->LrBuffDA;
}

void ULrCommonLibrary::PrintLog(const UObject* WorldContextObject, const APlayerController* PC)
{
	const UWorld* World = Cast<UWorld>(WorldContextObject);
	if (PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Authority=%d Local=%d NetMode=%d"), PC->HasAuthority(), (PC->IsLocalController()), (int32)World->GetNetMode());
	}
	if (World->IsNetMode(NM_ListenServer))
	{
		UE_LOG(LogTemp, Warning, TEXT("ListenServer"));
	}
	else if (World->IsNetMode(NM_Client))
	{
		UE_LOG(LogTemp, Warning, TEXT("Client"));
	}
	else if (World->IsNetMode(NM_DedicatedServer))
	{
		UE_LOG(LogTemp, Warning, TEXT("DedicatedServer"));
	}
}
