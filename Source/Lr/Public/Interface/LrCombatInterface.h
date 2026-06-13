// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LrCombatInterface.generated.h"

class ULrAS;
class ULrASC;
class ULrCombatComponentBase;


USTRUCT(BlueprintType)
struct FLrDieParameters
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Corpse")
	FVector DeathImpulse = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Corpse")
	float Duration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Corpse")
	uint32 PawnType = 0; //生物类型

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> DeathMontage; //死亡动画
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULrCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LR_API ILrCombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// virtual ULrASC* GetAbilitySystemComponent() const;
	virtual ULrCombatComponentBase* GetCombatComponent() const =0;
	virtual uint8 GetTeamID() const = 0;
	virtual bool IsDead() const = 0;
	virtual ULrASC* GetASC() const =0;
	virtual ULrAS* GetAS() const =0;

	//去死 可能会倒飞出去
	virtual void ToDie(const FLrDieParameters& LrDieConfig) = 0;
};
