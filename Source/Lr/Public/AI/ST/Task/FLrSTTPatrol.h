#pragma once

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "StateTreeTaskBase.h"
#include "AI/Controller/LrAIControllerBase.h"
#include "Component/LrPatrolRouteComponent.h"
#include "Engine/World.h"
#include "Pawn/LrEnemyPawn.h"
#include "FLrSTTPatrol.generated.h"

/**
 *Running, // 当前节点或状态仍在执行中，StateTree 会继续调用 Tick
 *Stopped,// StateTree 被外部请求停止，但没有明确成功或失败
 *Succeeded,// 当前节点或状态执行完成且成功，可能触发状态切换或终止
 *Failed,// 当前节点或状态执行失败，可能触发状态切换或终止
 *Unset, // 默认值，状态未初始化
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 巡逻
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
USTRUCT()
struct FPatrolTaskInstanceData
{
	GENERATED_BODY()

	// 当前目标巡逻点
	UPROPERTY(EditAnywhere, Category="Output")
	FVector CurrentDestination = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FLrPatrolTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	using FInstanceDataType = FPatrolTaskInstanceData;

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
		ULrPatrolRouteComponent* Patrol = Enemy.GetPatrolRoute();
		if (!Patrol)
		{
			return EStateTreeRunStatus::Failed;
		}
		const FVector NextPoint = Patrol->GetNextPatrolPoint();
		InstanceData.CurrentDestination = NextPoint;
		if (NextPoint.IsNearlyZero())
		{
			return EStateTreeRunStatus::Failed;
		}
		ALrAIControllerBase* AI = Cast<ALrAIControllerBase>(Enemy.GetController());
		if (!AI)
		{
			return EStateTreeRunStatus::Failed;
		}
		// 移动
		AI->MoverToTarget(InstanceData.CurrentDestination);
		return EStateTreeRunStatus::Running;
	}

	// 检查是否到达
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override
	{
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		ALrEnemyPawn& Enemy = Context.GetExternalData(LrEnemyPawn);
		FVector EnemyLocation = Enemy.GetActorLocation();
		const float Dist = FVector::Dist(Enemy.GetActorLocation(), InstanceData.CurrentDestination);
		ULrPatrolRouteComponent* Patrol = Enemy.GetPatrolRoute();
		if (!Patrol)
		{
			return EStateTreeRunStatus::Failed;
		}
		const FVector NextPoint = Patrol->GetNextPatrolPoint();
		ALrAIControllerBase* AI = Cast<ALrAIControllerBase>(Enemy.GetController());
		if (!AI)
		{
			return EStateTreeRunStatus::Failed;
		}
		if (AI->MoveState == EMoveState::Finish)
		{
			InstanceData.CurrentDestination = NextPoint;
			// 继续移动
			AI->MoverToTarget(InstanceData.CurrentDestination);
			return EStateTreeRunStatus::Running;
		}

		if (Patrol->TargetActor != nullptr)
		{
			return EStateTreeRunStatus::Succeeded;
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
