// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Interface/LrEquipInterface.h"
#include "Pawn/LrPawnBase.h"
#include "LrHeroPawn.generated.h"

class UCapsuleComponent;
class UAIPerceptionStimuliSourceComponent;
class UAIPerceptionComponent;
class UNiagaraSystem;
class ALrWeaponBase;
class UGameplayAbility;
class USpringArmComponent;
class UCameraComponent;
/**
 * 
 */
UCLASS()
class LR_API ALrHeroPawn : public ALrPawnBase, public ILrEquipInterface
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
	// 装备武器
	virtual void EquipWeapon(FLrWeaponConfig WeaponConfig) override;
	virtual void Unequipped(FLrWeaponConfig WeaponConfig) override;
	
	/** 碰撞体（NavAgent & Mover 都依赖它） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<UCapsuleComponent> LrCapsuleComponent;

	/** 相机 弹簧臂*/
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;
	/** 相机 弹簧臂*/

	/** 感知组件 */
	//敌我区分
	UPROPERTY(VisibleAnywhere, Category="Team")
	TObjectPtr<UAIPerceptionStimuliSourceComponent>StimuliSource;
	/** 感知组件 */
	
private:
	// 技能列表
	// UPROPERTY(EditAnywhere, Category="Abilities")
	// TArray<FGameplayTag> GATagListConfig;
};
