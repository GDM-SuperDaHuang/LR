#pragma once
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "StateTreeTaskBase.h"
#include "AI/Controller/LrAIControllerBase.h"
#include "AI/ST/LrSTComponent.h"
#include "Component/LrPatrolRouteComponent.h"
#include "Component/Combat/LrAICombatComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Pawn/LrEnemyPawn.h"
#include "FLrSTT.generated.h"

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
				AI->MoveToPlayer(Patrol->TargetActor);
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
				AI->MoveToPlayer(InstanceData.TargetActor);
			}
			return EStateTreeRunStatus::Running;
		}
		return EStateTreeRunStatus::Succeeded;
	}

protected:
	// 声明 Handle
	TStateTreeExternalDataHandle<ALrEnemyPawn> LrEnemyPawn;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 技能
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
USTRUCT()
struct FLrActivateAbilityTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, Category="Setting")
	float AttackRange = 120.f; //根据距离
};

USTRUCT(meta=(DisplayName="ActivateAbility"))
struct FLrActivateAbilityTask : public FStateTreeTaskCommonBase
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

		ULrAICombatComponent* Combat = Enemy.FindComponentByClass<ULrAICombatComponent>();
		if (!Combat)
		{
			return EStateTreeRunStatus::Failed;
		}

		//是否可以攻击
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
		if (!Combat->CanAttack(InstanceData.TargetActor))
		{
			return EStateTreeRunStatus::Failed;
		}
		//是否开始攻击
		if (!Combat->StartAttack())
		{
			return EStateTreeRunStatus::Failed;
		}
		return EStateTreeRunStatus::Succeeded;
	}

protected:
	// 声明 Handle
	TStateTreeExternalDataHandle<ALrEnemyPawn> LrEnemyPawn;
};
