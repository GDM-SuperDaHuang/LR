#pragma once
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "StateTreeTaskBase.h"
#include "AI/ST/LrSTComponent.h"
#include "Pawn/LrEnemyPawn.h"
#include "FLrSTT.generated.h"

/**
 *Running, // 当前节点或状态仍在执行中，StateTree 会继续调用 Tick
 *Stopped,// StateTree 被外部请求停止，但没有明确成功或失败
 *Succeeded,// 当前节点或状态执行完成且成功，可能触发状态切换或终止
 *Failed,// 当前节点或状态执行失败，可能触发状态切换或终止
 *Unset, // 默认值，状态未初始化
 */
USTRUCT()
struct FPatrolTaskInstanceData
{
	GENERATED_BODY()

	// 当前目标巡逻点
	UPROPERTY()
	FVector CurrentDestination = FVector::ZeroVector;
};

// 1. Task 实例数据
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
		ALrEnemyPawn& Data = Context.GetExternalData(LrEnemyPawn);
		
		//读取
		
		// if (Data.EnemyData.PatrolPoints.Num() == 0)
		// {
		// 	return EStateTreeRunStatus::Failed;
		// }
		// InstanceData.CurrentDestination = Data.EnemyData.PatrolPoints[Data.EnemyData.CurrentPatrolIndex];
		return EStateTreeRunStatus::Running;
	}

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override
	{
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		// ULrSTComponent& Data = Context.GetExternalData(LrSTComponent);
		//
		// if (!Data.EnemyData.SelfPawn)
		// 	return EStateTreeRunStatus::Failed;
		//
		// const FVector CurrentLoc = Data.EnemyData.SelfPawn->GetActorLocation();
		// if (FVector::Dist(CurrentLoc, InstanceData.CurrentDestination) < 100.f)
		// {
		// 	// 切换到下一个巡逻点
		// 	Data.EnemyData.CurrentPatrolIndex = (Data.EnemyData.CurrentPatrolIndex + 1) % Data.EnemyData.PatrolPoints.Num();
		// 	InstanceData.CurrentDestination = Data.EnemyData.PatrolPoints[Data.EnemyData.CurrentPatrolIndex];
		// }
		//
		// // AI 移动
		// Data.EnemyData.SelfPawn->AddMovementInput((InstanceData.CurrentDestination - CurrentLoc).GetSafeNormal());
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

// 移动
// USTRUCT(BlueprintType, meta=(DisplayName="Move To Target"))
// struct FLrMoveToTargetTask : public FStateTreeTaskCommonBase
// {
// 	GENERATED_BODY()
//
// 	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override
// 	{
// 		return EStateTreeRunStatus::Running;
// 	}
// };

// // 追
// USTRUCT()
// struct FLrChaseTaskInstanceData
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere)
// 	TObjectPtr<AActor> TargetActor;
// };
//
// USTRUCT(meta=(DisplayName="Chase"))
// struct FLrChaseTask : public FStateTreeTaskCommonBase
// {
// 	GENERATED_BODY()
//
// 	using FInstanceDataType = FLrChaseTaskInstanceData;
//
// 	virtual const UStruct* GetInstanceDataType() const override
// 	{
// 		return FInstanceDataType::StaticStruct();
// 	}
//
// 	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override
// 	{
// 		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
//
// 		if (!InstanceData.TargetActor)
// 		{
// 			return EStateTreeRunStatus::Failed;
// 		}
//
// 		return EStateTreeRunStatus::Running;
// 	}
// };
