// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/LrGameInstance.h"

#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Save/LrSaveGame.h"

void ULrGameInstance::Init()
{
	Super::Init();
	LoadInputSave();
}

void ULrGameInstance::LoadInputSave()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("InputSave"), 0))
	{
		InputSaveGame = Cast<ULrSaveGame>(
			UGameplayStatics::LoadGameFromSlot(
				TEXT("InputSave"),
				0));
	}
	else
	{
		InputSaveGame = Cast<ULrSaveGame>(
			UGameplayStatics::CreateSaveGameObject(
				ULrSaveGame::StaticClass()));

		SaveInput();
	}
}

void ULrGameInstance::SaveInput()
{
	if (InputSaveGame)
	{
		UGameplayStatics::SaveGameToSlot(InputSaveGame,TEXT("InputSave"), 0);
	}
}

void ULrGameInstance::RebindKey(int32 InputId, FKey NewKey)
{
	if (!InputSaveGame)
	{
		return;
	}

	FLrPlayerKeyMapping* Mapping =
		InputSaveGame->KeyMappings.FindByPredicate(
			[&](const FLrPlayerKeyMapping& Data)
			{
				return Data.InputId == static_cast<EGAInputID>(InputId);
			});

	if (Mapping)
	{
		Mapping->CustomKeys.Empty();
		Mapping->CustomKeys.Add(NewKey);
	}
	else
	{
		FLrPlayerKeyMapping NewMapping;
		
		NewMapping.InputId = static_cast<EGAInputID>(InputId);
		NewMapping.CustomKeys.Add(NewKey);

		InputSaveGame->KeyMappings.Add(NewMapping);
	}

	SaveInput();
}

void ULrGameInstance::RefreshInputBind(UInputMappingContext* MappingContext, const ULrInputConfigDA* InputConfig) const
{
	ULrInputComponent::ApplyPlayerKeyMappings(InputConfig, InputSaveGame, MappingContext);
}
