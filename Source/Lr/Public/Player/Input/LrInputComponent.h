// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Data/LrInputConfigDA.h"
#include "Save/LrSaveGame.h"
#include "LrInputComponent.generated.h"

struct FEnhancedActionKeyMapping;
/**
 * 
 */
UCLASS()
class LR_API ULrInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityActions(const ULrInputConfigDA* InputConfig, UserClass* Object, UInputMappingContext* MappingContext, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);

	//工具函数 修改键位
	static void ApplyPlayerKeyMappings(const ULrInputConfigDA* InputConfig, const ULrSaveGame* SaveGame, UInputMappingContext* MappingContext);

};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void ULrInputComponent::BindAbilityActions(const ULrInputConfigDA* InputConfig,
                                           UserClass* Object,
                                           UInputMappingContext* MappingContext,
                                           PressedFuncType PressedFunc,
                                           ReleasedFuncType ReleasedFunc,
                                           HeldFuncType HeldFunc)
{
	check(InputConfig);
	check(MappingContext);
	// 解绑所有
	MappingContext->UnmapAll();
	// 解绑所有
	// TArray<FEnhancedActionKeyMapping> OldMappings = MappingContext->GetMappings();
	// for (const auto& Mapping : OldMappings)
	// {
	// 	MappingContext->UnmapKey(
	// 		Mapping.Action,
	// 		Mapping.Key
	// 	);
	// }
	for (const FLrDefaultBindInputFKey& BindInfo : InputConfig->LrBindInputFKeyList)
	{
		if (!BindInfo.InputAction)
			continue;
		// 动态绑定按键
		// for (const FKey& Key : BindInfo.BoundKeys)
		// {
		// 	if (Key.IsValid())
		// 	{
		// 		MappingContext->MapKey(BindInfo.InputAction, Key);
		// 	}
		// }
		
		// 按键绑定触发函数
		if (PressedFunc)
		{
			// BindAction(BindInfo.InputAction, ETriggerEvent::Started, Object, PressedFunc, BindInfo.InputTag);
			BindAction(BindInfo.InputAction, ETriggerEvent::Started, Object, PressedFunc, BindInfo.InputId);

		}
		if (ReleasedFunc)
		{
			// BindAction(BindInfo.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, BindInfo.InputTag);
			BindAction(BindInfo.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, BindInfo.InputId);

		}

		if (HeldFunc)
		{
			// BindAction(BindInfo.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, BindInfo.InputTag);
			BindAction(BindInfo.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, BindInfo.InputId);
		}
	}
}
