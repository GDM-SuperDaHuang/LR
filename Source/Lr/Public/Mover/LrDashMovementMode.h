// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mover/LrBaseMovementMode.h"
#include "LrDashMovementMode.generated.h"

namespace ExtendedModeNames
{
	static const FName Dash = TEXT("Dash");
}


// USTRUCT()
// struct FDashState : public FMoverDataStructBase
// {
// 	GENERATED_BODY()

// 	float RemainingTime = 0.f;

// 	virtual FMoverDataStructBase* Clone() const override
// 	{
// 		return new FDashState(*this);
// 	}

// 	virtual bool NetSerialize(FArchive& Ar, UPackageMap*, bool& bOutSuccess) override
// 	{
// 		Super::NetSerialize(Ar, nullptr, bOutSuccess);
// 		Ar << RemainingTime;
// 		bOutSuccess = true;
// 		return true;
// 	}

// 	virtual UScriptStruct* GetScriptStruct() const override
// 	{
// 		return StaticStruct();
// 	}
// };

/**
 * Dash 是：典型“时间驱动 Mode”
 * 短时间，
 * 固定方向，
 * 固定速度，
 * 自动结束
 * 
 	Dash 输入	FMoverExampleAbilityInputs
	Dash 状态	FDashState（SyncState）
	位移	SimulationTick
	结束	Duration 到期 → 切 Mode
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


USTRUCT()
struct FDashState : public FMoverDataStructBase
{
	GENERATED_USTRUCT_BODY()

	// Dash direction (world space, normalized)
	FVector DashDirection = FVector::ZeroVector;

	// Elapsed time
	float ElapsedTime = 0.f;

	// Total duration
	float Duration = 0.f;

	virtual FMoverDataStructBase* Clone() const override
	{
		return new FDashState(*this);
	}

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override
	{
		Super::NetSerialize(Ar, Map, bOutSuccess);

		Ar << DashDirection;
		Ar << ElapsedTime;
		Ar << Duration;

		bOutSuccess = true;
		return true;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override
	{
		const FDashState& Auth = static_cast<const FDashState&>(AuthorityState);

		return !DashDirection.Equals(Auth.DashDirection)
			|| !FMath::IsNearlyEqual(ElapsedTime, Auth.ElapsedTime)
			|| !FMath::IsNearlyEqual(Duration, Auth.Duration);
	}

	virtual void Interpolate(
		const FMoverDataStructBase& From,
		const FMoverDataStructBase& To,
		float Pct) override
	{
		const FDashState& A = static_cast<const FDashState&>(From);
		const FDashState& B = static_cast<const FDashState&>(To);

		DashDirection = B.DashDirection;
		ElapsedTime = FMath::Lerp(A.ElapsedTime, B.ElapsedTime, Pct);
		Duration = B.Duration;
	}
};

template<>
struct TStructOpsTypeTraits<FDashState> : public TStructOpsTypeTraitsBase2<FDashState>
{
	enum { WithCopy = true };
};
