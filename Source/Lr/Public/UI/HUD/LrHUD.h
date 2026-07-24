// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASC/LrASC.h"
#include "GameFramework/HUD.h"
#include "LrHUD.generated.h"

class ULrCrosshairWidget;
class ULrWorldUIRoot;
class ULrMPBarWidget;
class ULrHPBarWidget;
class ULrUIController;
class ULrMVVMVBar;
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

	void HandleASCRegistered(ULrASC* LrAsc);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// void HandleEquipRequest(ALrWeaponBase LrWeaponBase);

	UPROPERTY()
	TObjectPtr<UMVVMMainScreen> ViewModel;

	UPROPERTY()
	ULrMVVMVBar* HPViewModel;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULrCrosshairWidget> CrosshairClass;
	UPROPERTY()
	TObjectPtr<ULrCrosshairWidget> Crosshair;
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


private:
	UPROPERTY()
	TObjectPtr<ULrUIController> UIController;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULrHPBarWidget> HPWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULrMPBarWidget> MPWidgetClass;
	
public:
	// 敌人UI
	 // UPROPERTY(EditDefaultsOnly)
	 // TSubclassOf<UUserWidget> WorldUIRootClass;
	 // UPROPERTY()
	 // ULrWorldUIRoot* WorldUIRoot;
	/** AI相关 */ 
	// FORCEINLINE
	// ULrWorldUIRoot* CreateWorldUIRoot();
};
