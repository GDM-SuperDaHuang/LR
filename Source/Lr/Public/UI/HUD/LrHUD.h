// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LrHUD.generated.h"

struct FLrWeaponConfig;
class ALrWeaponBase;
class ULrMainWidget;
class UMVVMMainScreen;
/**
 * HUD 不负责按钮逻辑、不负责装备、不负责 UI 细节。只加载哪个 UI
 */
UCLASS()
class LR_API ALrHUD : public AHUD
{
	GENERATED_BODY()

public:
	void HandleEquipRequest(FLrWeaponConfig WeaponConfig);
	void HandleUnequipRequest(FLrWeaponConfig WeaponConfig);

	virtual void BeginPlay() override;
	// void HandleEquipRequest(ALrWeaponBase LrWeaponBase);

	UPROPERTY()
	TObjectPtr<UMVVMMainScreen> ViewModel;

protected:

private:
	// 按钮控制器
	// UPROPERTY(EditDefaultsOnly)
	// TSubclassOf<UMVVMMainScreen> ViewModelClass;


	// 按钮UI
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULrMainWidget> MainWidgetClass;
	UPROPERTY()
	TObjectPtr<ULrMainWidget> MainWidget;
};
