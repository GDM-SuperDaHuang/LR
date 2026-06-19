// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LrCombatComponentBase.h"
#include "LrPlayerCombatComponent.generated.h"

class ALrPawnBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrPlayerCombatComponent : public ULrCombatComponentBase
{
	GENERATED_BODY()

public:
	ULrPlayerCombatComponent();

	// 新增：手动选择一个目标
	virtual TWeakObjectPtr<ALrPawnBase> GetNearestPawnToCursor();
	
};
