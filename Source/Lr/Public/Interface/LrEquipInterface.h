// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LrEquipInterface.generated.h"

struct FLrWeaponConfig;
class ALrWeaponBase;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULrEquipInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LR_API ILrEquipInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void EquipWeapon(FLrWeaponConfig WeaponConfig);
	virtual void Unequipped(FLrWeaponConfig WeaponConfig);
};
