// Fill out your copyright notice in the Description page of Project Settings.


#include "LrAssetManager.h"

#include "Tags/LrGameplayTags.h"

ULrAssetManager& ULrAssetManager::Get()
{
	check(GEngine);
	ULrAssetManager* AuraAssetManager = Cast<ULrAssetManager>(GEngine->AssetManager);

	return *AuraAssetManager;
}

void ULrAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	FLrGameplayTags::InitializeNativeGameplayTags();

}
