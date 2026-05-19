// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "LrBTServiceUpdateCombat.generated.h"

/**
 * 战斗状态更新服务
 * 定期更新黑板中的 DistanceToTarget 和 CombatState 键值
 * 行为树 Decorator 可读取这些值决定是否进入攻击分支
 *
 * 工作逻辑：
 * 1. 从黑板读取 TargetActor（由 LrAIController::OnTargetDetected 写入）
 * 2. 计算自身与目标的距离，写入 DistanceToTarget 键
 * 3. 判断距离是否 <= AttackRange，结果写入 CombatState 键
 */
UCLASS()
class LR_API ULrBTServiceUpdateCombat : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	ULrBTServiceUpdateCombat();

protected:
	/**
	 * 每 0.2 秒执行一次
	 * 计算到目标的距离，更新黑板中的战斗相关键值
	 */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	/** 判定可攻击的最大距离（单位：厘米，默认 250 = 2.5m） */
	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float AttackRange = 250.f;
};