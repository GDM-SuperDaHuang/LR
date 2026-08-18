#pragma once

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "StateTreeTaskBase.h"
#include "AI/Controller/LrAIControllerBase.h"
#include "Component/LrPatrolRouteComponent.h"
#include "Component/Combat/LrAICombatComponent.h"
#include "Engine/World.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMoverComponent.h"
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

	UPROPERTY(EditDefaultsOnly, Category="Setting")
	float AttackCooldown = 1.5f; //冷却时间
};

USTRUCT(meta=(DisplayName="ActivateAbility"))
struct FLrActivateAbilityTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FLrActivateAbilityTaskInstanceData;

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
		if (!Combat->CanAttack(InstanceData.TargetActor, InstanceData.AttackRange, InstanceData.AttackCooldown))
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
