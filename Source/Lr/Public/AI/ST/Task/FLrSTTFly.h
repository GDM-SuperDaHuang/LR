#pragma once

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "StateTreeTaskBase.h"
#include "AI/Controller/LrAIControllerBase.h"
#include "Engine/World.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMoverComponent.h"
#include "Pawn/LrEnemyPawn.h"
#include "FLrSTTFly.generated.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 飞行初始化
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
USTRUCT()
struct FFlyInitTaskInstanceData
{
	GENERATED_BODY()
	// 飞行高度
	UPROPERTY(EditAnywhere, Category="Setting")
	float FlyHeight = 70.f;

	UPROPERTY(EditAnywhere, Category="Output")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Output")
	FVector InitLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FLrFlyInitTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	using FInstanceDataType = FFlyInitTaskInstanceData;
	using FExecutionRuntimeDataType = FFlyInitTaskInstanceData; // ← 新增

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual const UStruct* GetExecutionRuntimeDataType() const override // ← 新增
	{
		return FExecutionRuntimeDataType::StaticStruct();
	}


	// 在 Link 阶段绑定 ExternalData
	virtual bool Link(FStateTreeLinker& Linker) override
	{
		Linker.LinkExternalData(LrEnemyPawn);
		return true;
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override
	{
		// 初始化巡逻点
		// FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		FExecutionRuntimeDataType& InstanceData = Context.GetExecutionRuntimeData(*this); //持久数据

		// 获取 ExternalData
		ALrEnemyPawn& Enemy = Context.GetExternalData(LrEnemyPawn);

		ALrAIControllerBase* AI = Cast<ALrAIControllerBase>(Enemy.GetController());
		if (!AI)
		{
			return EStateTreeRunStatus::Failed;
		}

		FVector CurLocation = Enemy.GetActorLocation();
		CurLocation.Z = CurLocation.Z + InstanceData.FlyHeight;
		if (InstanceData.TargetLocation == FVector::ZeroVector)
		{
			// InstanceData.InitLocation = CurLocation;
			InstanceData.TargetLocation = CurLocation;
		}
		else
		{
			return EStateTreeRunStatus::Succeeded;
		}

		Enemy.LrMoverComponent->QueueNextMode(LrAllModes::Fly);
		// 移动
		AI->MoverToTarget(InstanceData.TargetLocation, 1);
		return EStateTreeRunStatus::Running;
	}

	// 检查是否到达
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override
	{
		// FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		FExecutionRuntimeDataType& InstanceData = Context.GetExecutionRuntimeData(*this); //持久数据

		ALrEnemyPawn& Enemy = Context.GetExternalData(LrEnemyPawn);
		//射线检测
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(&Enemy);
		const FVector ProbeStart = Enemy.GetActorLocation();
		const FVector ProbeEnd = ProbeStart + FVector::DownVector * 10000.f;

		FHitResult ProbeHit;
		const bool bProbeHit = Enemy.GetWorld()->LineTraceSingleByChannel(ProbeHit, ProbeStart, ProbeEnd, ECC_WorldStatic, QueryParams);
		if (bProbeHit)
		{
			float HeightFromStart = ProbeStart.Z - ProbeHit.Location.Z;
			if (HeightFromStart > InstanceData.FlyHeight)
			{
				return EStateTreeRunStatus::Succeeded;
			}
			else
			{
				ALrAIControllerBase* AI = Cast<ALrAIControllerBase>(Enemy.GetController());
				if (!AI)
				{
					return EStateTreeRunStatus::Failed;
				}
				// Only retry after an explicit completed request. A failed synchronous
				// query also leaves PathFollowing idle and must not spin here.
				if (AI->MoveState == EMoveState::Finish && AI->GetMoveStatus() == EPathFollowingStatus::Idle)
				{
					AI->MoverToTarget(InstanceData.TargetLocation,1);
				}
				return EStateTreeRunStatus::Running;
			}
		}
		return EStateTreeRunStatus::Running;
	}

	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override
	{
		// 离开状态时，停止移动
		// StopMovement();
	}

protected:
	// 声明 Handle
	TStateTreeExternalDataHandle<ALrEnemyPawn> LrEnemyPawn;
};
