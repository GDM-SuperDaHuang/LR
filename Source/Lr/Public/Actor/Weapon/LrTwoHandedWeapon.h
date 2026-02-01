// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Weapon/LrWeaponBase.h"
#include "Pawn/LrPawnBase.h"
#include "LrTwoHandedWeapon.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ALrTwoHandedWeapon : public ALrWeaponBase
{
	GENERATED_BODY()

public:
	ALrTwoHandedWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName HandSocketLName = TEXT("FX_WeaponBase_L"); // 左手 Socket
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName HandSocketRName = TEXT("FX_WeaponBase_R"); // 右手 Socket

	/** 武器 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<USkeletalMeshComponent> WeaponLSKM;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<USkeletalMeshComponent> WeaponRSKM;
	/** 武器 */


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<UNiagaraComponent> WeaponRComponent;

protected:
	virtual void OnEquipped(AActor* OwnerPawn, FLrWeaponConfig WeaponConfig) override;
	virtual void OnUnequipped() override;
};
