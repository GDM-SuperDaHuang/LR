// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mover/LrBaseMovementMode.h"
#include "LrDashMovementMode.generated.h"

namespace ExtendedModeNames
{
	static const FName Dash = TEXT("Dash");
}


/**
 * Dash 是：典型“时间驱动 Mode”
 * 短时间，
 * 固定方向，
 * 固定速度，
 * 自动结束
 * 
 	Dash 输入	ULrDashMovementMode
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

	/**
	 * FMoverDataStructBase
	 * Input State (刺激) 原始 Enhanced Input
	 * Sync  State (权威) 上一帧权威状态
	 * Aux   State (表现) 上一帧权威状态
	 */
	virtual void OnGenerateMove(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
	virtual void OnSimulationTick(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;

};

/**
 * FMoverDataStructBase的结构体用于封装所需的输入参数
 * FMoverDataStructBase 是 Mover 框架中
 * “可预测、可回滚、可插值、值语义”的最小状态单元。
 */
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

	// 复制历史帧
	// 帧结束：调用 历史快照（Clone）
	virtual FMoverDataStructBase* Clone() const override
	{
		return new FDashState(*this);
	}


	//精准同步
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

	// 判断是否回滚
	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override
	{
		const FDashState& Auth = static_cast<const FDashState&>(AuthorityState);

		return !DashDirection.Equals(Auth.DashDirection)
			|| !FMath::IsNearlyEqual(ElapsedTime, Auth.ElapsedTime)
			|| !FMath::IsNearlyEqual(Duration, Auth.Duration);
	}

	// 客户端平滑
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
