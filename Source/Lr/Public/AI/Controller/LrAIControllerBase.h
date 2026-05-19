// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "LrAIControllerBase.generated.h"

class UBehaviorTreeComponent;
struct FAIStimulus;
class UAISenseConfig_Sight;

/**
*if (HealthPercent <= 0.5f)
{
	AISubsystem->SwitchToBossAI(this);
}
 * 自定义 AI 控制器
 * 负责管理 AI 的感知系统和行为树，处理目标检测与追击逻辑
 *
 * 工作流程：
 * 1. 构造时创建黑板组件、感知组件和视觉配置
 * 2. OnPossess 时根据 EnemyPawn 的行为树资产启动行为树，并绑定感知回调
 * 3. OnTargetDetected 将感知到的目标 Actor 写入/清除黑板的 TargetActor 键
 * 4. 行为树通过 BTService 和 BTTask 读取黑板数据进行决策
 */
UCLASS()
class LR_API ALrAIControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	ALrAIControllerBase();
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void ShutdownAI();

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBehaviorTree> LrBehaviorTree;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBlackboardComponent> LrBlackboardComponent;

	// UPROPERTY(EditDefaultsOnly)
	// TObjectPtr<UBehaviorTreeComponent> LrBehaviorTreeComponent;
protected:
	/**
	 * 接管 Pawn 时的初始化逻辑
	 * 1. 从 EnemyPawn 获取行为树资产并运行
	 * 2. 缓存黑板组件引用，供感知回调和任务节点使用
	 * 3. 绑定 OnTargetPerceptionUpdated 委托，监听视野内的目标变化
	 */
	virtual void OnPossess(APawn* InPawn) override;
	virtual void InitializeAI();
	
	/**
	 * 感知系统检测到目标的回调
	 * @param Actor   被感知到的 Actor
	 * @param Stimulus 感知刺激数据（包含 WasSuccessfullySensed 判断是进入视野还是离开视野）
	 *
	 * 当目标进入感知范围时，写入黑板的 TargetActor 键
	 * 当目标离开感知范围时，清除黑板的 TargetActor 键
	 */
	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);
	
	virtual void Tick(float DeltaSeconds) override;
	void StartChase(AActor* InTarget);

	void StopChase();

protected:
	UPROPERTY(EditDefaultsOnly, Category="Team")
	uint8 TeamID = 1;
	
	// UPROPERTY()
	// TObjectPtr<AActor> CurrentTarget;
	//
	// bool bIsChasing = false;
	UPROPERTY(EditDefaultsOnly)
	float AcceptanceRadius = 120.f;
	/**
	 * 视觉感知配置
	 * 包含视野半径（SightRadius=2000）、丢失半径（LoseSightRadius=2500）、
	 * 视野角度（PeripheralVisionAngleDegrees=90）和记忆时长（MaxAge=5s）
	 */
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
};