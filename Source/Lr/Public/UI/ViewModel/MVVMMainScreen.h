// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Data/LrGAListDA.h"
#include "UI/Widget/Button/LrButtonInfo.h"
#include "MVVMMainScreen.generated.h"

class ALrWeaponBase;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquipRequest,FLrWeaponConfig);

/**
 * 
 */
UCLASS()
class LR_API UMVVMMainScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:

	//  是否穿上
	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "State")
	bool bWeaponEquipped = false;
	
	// 点击穿上
	void Unequipped(const FLrWeaponConfig& WeaponConfig);
	void EquipWeapon(const FLrWeaponConfig& WeaponConfig);
	// void EquipWeapon(const TSubclassOf<class ALrWeaponBase>& WeaponClass);
	

	FOnEquipRequest OnEquipRequest;
	FOnEquipRequest OnUnequipRequest;

};
