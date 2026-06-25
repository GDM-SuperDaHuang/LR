// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Input/LrInputComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Save/LrSaveGame.h"
#include "Tags/LrGameplayTags.h"

void ULrInputComponent::ApplyPlayerKeyMappings(const ULrInputConfigDA* InputConfig, const ULrSaveGame* SaveGame, UInputMappingContext* MappingContext)
{
	if (!InputConfig || !MappingContext)
	{
		return;
	}

	// 清空旧映射
	MappingContext->UnmapAll();

	// 浮点数，技能按键相关
	for (const FLrDefaultBindInputFKey& Row : InputConfig->LrBindInputFKeyList)
	{
		if (!Row.InputAction)
		{
			continue;
		}
		// 查找玩家自定义按键
		const FLrPlayerKeyMapping* PlayerMapping = SaveGame->KeyMappings.FindByPredicate(
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

	//移动操作wasd
	for (const FLrAxisBindInputFKey& Axis : InputConfig->LrAxisBindInputFKeyList)
	{
		if (!Axis.InputAction)
		{
			continue;
		}
		for (const FLrAxisKey& Key : Axis.LrAxisKeyList)
		{
			FEnhancedActionKeyMapping& Mapping = MappingContext->MapKey(Axis.InputAction, Key.BoundKey);
			// ✔ 关键点：Scalar 直接表达方向
			UInputModifierScalar* Scalar = NewObject<UInputModifierScalar>();

			// ✔ 关键：不同 Axis 控制不同维度
			if (Key.IsFU)
			{
				UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>();
				Swizzle->Order = EInputAxisSwizzle::YXZ;
				Mapping.Modifiers.Add(Swizzle);
			}

			// --------------------------------
			// 方向
			// --------------------------------
			if (Key.Scale < 0.f)
			{
				UInputModifierNegate* Negate = NewObject<UInputModifierNegate>();
				// 前后移动反转 Y
				if (Key.IsFU)
				{
					Negate->bY = true;
					Negate->bX = false;
				}
				else
				{
					Negate->bX = true;
					Negate->bY = false;
				}
				Mapping.Modifiers.Add(Negate);
			}
		}
	}
}
