// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mover/LrBaseMovementMode.h"
#include "LrDashMovementMode.generated.h"


USTRUCT()
struct FDashState : public FMoverDataStructBase
{
	GENERATED_BODY()

	float RemainingTime = 0.f;

	virtual FMoverDataStructBase* Clone() const override
	{
		return new FDashState(*this);
	}

	virtual bool NetSerialize(FArchive& Ar, UPackageMap*, bool& bOutSuccess) override
	{
		Super::NetSerialize(Ar, nullptr, bOutSuccess);
		Ar << RemainingTime;
		bOutSuccess = true;
		return true;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}
};
/**
 * 
 */
UCLASS()
class LR_API ULrDashMovementMode : public ULrBaseMovementMode
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	float DashSpeed = 1200.f;

	UPROPERTY(EditAnywhere)
	float DashDuration = 0.25f;

	
	virtual void OnGenerateMove(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void OnSimulationTick(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;

};
