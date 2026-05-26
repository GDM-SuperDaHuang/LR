// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LrCombatComponentBase.h"
#include "LrPlayerCombatComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrPlayerCombatComponent : public ULrCombatComponentBase
{
	GENERATED_BODY()

public:
	ULrPlayerCombatComponent();
};
