// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "LrBTServiceUpdateCombat.generated.h"

/**
 * 战斗状态更新服务
 * 每帧更新黑板中的 DistanceToTarget 和 bCanAttack 值
 * 用于行为树中判断是否进入攻击状态
 */
UCLASS()
class LR_API ULrBTServiceUpdateCombat : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	ULrBTServiceUpdateCombat();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	/** 判定可攻击的最大距离（单位：厘米） */
	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float AttackRange = 250.f;
};