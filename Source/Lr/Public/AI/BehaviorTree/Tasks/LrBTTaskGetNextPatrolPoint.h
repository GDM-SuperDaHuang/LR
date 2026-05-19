// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LrBTTaskGetNextPatrolPoint.generated.h"

/**
 * 开始追击任务
 * 在 AI 位置周围 SearchRadius 范围内随机找到一个 NavMesh 可达点，
 * 写入黑板的 MoveLocation 键，供后续 MoveTo 任务使用
 *
 * 工作流程：
 * 1. 获取 AI 控制的 Pawn 位置
 * 2. 通过 NavigationSystemV1 在半径范围内查找随机可达点
 * 3. 将找到的位置写入黑板 MoveLocation
 */
UCLASS()
class LR_API ULrBTTaskGetNextPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULrBTTaskGetNextPatrolPoint();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};