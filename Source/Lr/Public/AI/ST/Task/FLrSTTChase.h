#pragma once

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "StateTreeTaskBase.h"
#include "AI/Controller/LrAIControllerBase.h"
#include "Component/LrPatrolRouteComponent.h"
#include "Engine/World.h"
#include "Navigation/PathFollowingComponent.h"
#include "Pawn/LrEnemyPawn.h"
#include "FLrSTTChase.generated.h"

/**
 *Running, // 当前节点或状态仍在执行中，StateTree 会继续调用 Tick
 *Stopped,// StateTree 被外部请求停止，但没有明确成功或失败
 *Succeeded,// 当前节点或状态执行完成且成功，可能触发状态切换或终止
 *Failed,// 当前节点或状态执行失败，可能触发状态切换或终止
 *Unset, // 默认值，状态未初始化
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 追
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
USTRUCT()
struct FLrChaseTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, Category="Setting")
	float AttackRange = 120.f; //根据距离

	UPROPERTY(EditAnywhere, Category="AI")
	float UpdateInterval = 0.5f; //更新频率

	UPROPERTY()
	float ElapsedTime = 0.f;
};

USTRUCT(meta=(DisplayName="Chase"))
struct FLrChaseTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FLrChaseTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

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

		if (!Patrol->TargetActor)
		{
			return EStateTreeRunStatus::Failed;
		}
		InstanceData.TargetActor = Patrol->TargetActor;
		// 移动
		const float DistSq = FVector::DistSquared(Enemy.GetActorLocation(), InstanceData.TargetActor->GetActorLocation());
		if (DistSq >= FMath::Square(InstanceData.AttackRange))
		{
			if (ALrAIControllerBase* AI = Cast<ALrAIControllerBase>(Enemy.GetController()))
			{
				// MoveToActor observes a moving goal and requests repaths itself. A
				// failed synchronous query also leaves PathFollowing idle, so only
				// retry after an explicit completed request.
				if (AI->MoveState == EMoveState::Finish && AI->GetMoveStatus() == EPathFollowingStatus::Idle)
				{
					AI->MoveToPlayer(Patrol->TargetActor);
				}
			}
		}
		return EStateTreeRunStatus::Running;
	}

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override
	{
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		InstanceData.ElapsedTime += DeltaTime;
		if (InstanceData.ElapsedTime < InstanceData.UpdateInterval)
		{
			return EStateTreeRunStatus::Running;
		}
		InstanceData.ElapsedTime = 0.f;
		ALrEnemyPawn& Enemy = Context.GetExternalData(LrEnemyPawn);
		FVector ActorLocation = InstanceData.TargetActor->GetActorLocation();
		const float DistSq = FVector::DistSquared(Enemy.GetActorLocation(), InstanceData.TargetActor->GetActorLocation());
		if (DistSq >= FMath::Square(InstanceData.AttackRange)) // todo
		{
			if (ALrAIControllerBase* AI = Cast<ALrAIControllerBase>(Enemy.GetController()))
			{
				// The active MoveToActor request already tracks the target actor.
				// Do not treat a failed query (Idle) as an immediate retry signal.
				if (AI->MoveState == EMoveState::Finish && AI->GetMoveStatus() == EPathFollowingStatus::Idle)
				{
					AI->MoveToPlayer(InstanceData.TargetActor);
				}
			}
			return EStateTreeRunStatus::Running;
		}
		return EStateTreeRunStatus::Succeeded;
	}

protected:
	// 声明 Handle
	TStateTreeExternalDataHandle<ALrEnemyPawn> LrEnemyPawn;
};
