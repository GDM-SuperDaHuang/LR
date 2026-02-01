// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/MVVMMainScreen.h"
/**
 *
 *ViewModel 不执行装备逻辑
 *ViewModel 只是说：
 *👉「我想装备武器了」
 */
void UMVVMMainScreen::EquipWeapon(const FLrWeaponConfig& WeaponConfig)
{
	if (bWeaponEquipped)
	{
		return;
	}
	// 通知外部（HUD / PC）
	OnEquipRequest.Broadcast(WeaponConfig);
	bWeaponEquipped = true;
	// 先更新 UI 状态（是否成功由外部决定也可以再回调）
	UE_MVVM_SET_PROPERTY_VALUE(bWeaponEquipped, true);
}

void UMVVMMainScreen::Unequipped(const FLrWeaponConfig& WeaponConfig)
{
	bWeaponEquipped = false;
	OnUnequipRequest.Broadcast(WeaponConfig);
	UE_MVVM_SET_PROPERTY_VALUE(bWeaponEquipped, false);
}
