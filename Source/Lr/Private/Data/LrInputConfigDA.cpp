// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/LrInputConfigDA.h"

const UInputAction* ULrInputConfigDA::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FLrInputAction& Action : AbilityInputActions)
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
