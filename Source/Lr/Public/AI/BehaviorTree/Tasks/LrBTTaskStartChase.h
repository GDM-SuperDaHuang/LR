// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LrBTTaskStartChase.generated.h"

/**
 * 开始追击任务
 * 在 AI 位置周围随机找到一个可达点，写入黑板的 MoveLocation 以供后续移动使用
 */
UCLASS()
class LR_API ULrBTTaskStartChase : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULrBTTaskStartChase();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	/** 随机搜索半径（单位：厘米） */
	UPROPERTY(EditDefaultsOnly, Category = "Chase", meta = (AllowPrivateAccess = "true"))
	float SearchRadius = 1000.f;
};