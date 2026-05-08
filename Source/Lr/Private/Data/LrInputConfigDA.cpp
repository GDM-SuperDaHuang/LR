// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/LrInputConfigDA.h"

#include "EnhancedActionKeyMapping.h"
#include "InputMappingContext.h"

const UInputAction* ULrInputConfigDA::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FLrDefaultBindInputFKey& Action : LrBindInputFKeyList)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find GA for Tag %s on InputConfig [%s]"), *InputTag.ToString(),*GetNameSafe(this));
	}
	return nullptr;
}

void ULrInputConfigDA::InitBindInputFKey(UInputMappingContext* IMC) const
{
	if (!IMC) return;

	for (const FLrDefaultBindInputFKey& BindInfo : LrBindInputFKeyList)
	{
		if (!BindInfo.InputAction) continue;

		for (const FKey& Key : BindInfo.BoundKeys)
		{
			FEnhancedActionKeyMapping Mapping;
			Mapping.Action = const_cast<UInputAction*>(BindInfo.InputAction);
			Mapping.Key = Key;
			// // 可选：设置触发条件（如按下、松开、按住）
			// Mapping.Triggers.Add(NewObject<UInputTriggerPressed>());
			// IMC->MapKey(BindInfo.InputAction,Key);
		}
	}
}
