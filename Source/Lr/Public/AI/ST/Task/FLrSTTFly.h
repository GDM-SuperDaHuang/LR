#pragma once

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "StateTreeTaskBase.h"
#include "AI/Controller/LrAIControllerBase.h"
#include "Engine/World.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMoverComponent.h"
#include "Pawn/LrEnemyPawn.h"
#include "FLrSTT.generated.h"
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
};

USTRUCT(BlueprintType)
struct FLrFlyInitTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	using FInstanceDataType = FFlyInitTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
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
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		// 获取 ExternalData
		ALrEnemyPawn& Enemy = Context.GetExternalData(LrEnemyPawn);
		FVector InitLocation = Enemy.GetActorLocation();
		InitLocation.Z = InitLocation.Z + InstanceData.FlyHeight;
		InstanceData.TargetLocation = InitLocation;
		ALrAIControllerBase* AI = Cast<ALrAIControllerBase>(Enemy.GetController());
		if (!AI)
		{
			return EStateTreeRunStatus::Failed;
		}
		Enemy.LrMoverComponent->QueueNextMode(LrAllModes::Fly);
		// 移动
		AI->MoverToTarget(InitLocation);
		return EStateTreeRunStatus::Running;
	}

	// 检查是否到达
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override
	{
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		ALrEnemyPawn& Enemy = Context.GetExternalData(LrEnemyPawn);
		//射线检测
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(&Enemy);
		const FVector ProbeStart = Enemy.GetActorLocation();
		const FVector ProbeEnd = ProbeStart + FVector::DownVector * 100.f;

		FHitResult ProbeHit;
		const bool bProbeHit = Enemy.GetWorld()->LineTraceSingleByChannel(ProbeHit, ProbeStart, ProbeEnd, ECC_WorldStatic, QueryParams);
		if (bProbeHit)
		{
			float HeightFromStart = ProbeStart.Z - ProbeHit.Location.Z;
			if (HeightFromStart < InstanceData.FlyHeight)
			{
				ALrAIControllerBase* AI = Cast<ALrAIControllerBase>(Enemy.GetController());
				if (!AI)
				{
					return EStateTreeRunStatus::Failed;
				}
				if (AI->MoveState == EMoveState::Finish)
				{
					// // 继续移动
					// AI->MoverToTarget(InstanceData.TargetLocation);
					return EStateTreeRunStatus::Succeeded;
				}
			}
			else
			{
				return EStateTreeRunStatus::Succeeded;
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
