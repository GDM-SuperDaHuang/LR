// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LrWeaponBase.generated.h"

struct FLrWeaponConfig;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class LR_API ALrWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALrWeaponBase();

	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	// FName HandSocketName = TEXT("FX_WeaponBase_L"); // 右手 Socket
	// // UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	// // FName HandSocketName = TEXT("FX_WeaponBase_R"); // 右手 Socket
	//
	// /** 武器 */
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PawnInfo")
	// TObjectPtr<USkeletalMeshComponent> WeaponSKM;
	// /** 武器 */

	/** 武器轨迹 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<UNiagaraComponent> WeaponComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<UNiagaraSystem> TrailNS; //可根据配置表动态加载设置
	/** 武器轨迹 */


	// =========================
	// 穿戴、拖下
	// =========================
	virtual void OnEquipped(AActor* OwnerPawn,FLrWeaponConfig WeaponConfig);
	virtual void OnUnequipped();
};
