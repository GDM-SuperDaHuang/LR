// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Input/LrInputComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Save/LrSaveGame.h"

void ULrInputComponent::ApplyPlayerKeyMappings(const ULrInputConfigDA* InputConfig, const ULrSaveGame* SaveGame, UInputMappingContext* MappingContext)
{
	if (!InputConfig || !MappingContext)
	{
		return;
	}

	// 清空旧映射
	MappingContext->UnmapAll();

	for (const FLrDefaultBindInputFKey& Row : InputConfig->LrBindInputFKeyList)
	{
		if (!Row.InputAction)
		{
			continue;
		}
		// 查找玩家自定义按键
		const FLrPlayerKeyMapping* PlayerMapping =
			SaveGame->KeyMappings.FindByPredicate(
				[&](const FLrPlayerKeyMapping& Data)
				{
					return Data.InputTag == Row.InputTag;
				});

		// 玩家自定义
		if (PlayerMapping)
		{
			for (const FKey& Key : PlayerMapping->CustomKeys)
			{
				MappingContext->MapKey(Row.InputAction, Key);
			}
		}
		else
		{
			// 默认键位
			for (const FKey& Key : Row.BoundKeys)
			{
				MappingContext->MapKey(Row.InputAction, Key);
			}
		}
	}
}
