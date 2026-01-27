// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pawn/LrPawnBase.h"
#include "LrHeroPawn.generated.h"

class UNiagaraComponent;
class UGameplayAbility;
class USpringArmComponent;
class UCameraComponent;
/**
 * 
 */
UCLASS()
class LR_API ALrHeroPawn : public ALrPawnBase
{
	GENERATED_BODY()

public:
	ALrHeroPawn();

	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	// 服务器
	virtual void PossessedBy(AController* NewController) override;

	// 客户端
	virtual void OnRep_PlayerState() override;

protected:
	/** 碰撞体（NavAgent & Mover 都依赖它） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<UCapsuleComponent> LrCapsuleComponent;

	/** 相机 弹簧臂*/
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;
	/** 相机 弹簧臂*/


	/** 武器 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<USkeletalMeshComponent> WeaponSKM;
	/** 武器 */


	/** 武器轨迹 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<UNiagaraComponent> WeaponTrailComponent;
	/** 武器轨迹 */
private:
	// 技能列表
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<FGameplayTag> GATagListConfig;
};
