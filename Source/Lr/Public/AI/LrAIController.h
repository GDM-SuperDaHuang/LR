// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "LrAIController.generated.h"

struct FAIStimulus;
class UAISenseConfig_Sight;

/**
 * 自定义 AI 控制器
 * 负责管理 AI 的感知系统和行为树，处理目标检测与追击逻辑
 */
UCLASS()
class LR_API ALrAIController : public AAIController
{
	GENERATED_BODY()

public:
	ALrAIController();

protected:
	/** 接管 Pawn 时的初始化逻辑：启动行为树并绑定感知回调 */
	virtual void OnPossess(APawn* InPawn) override;

	/** 当感知系统检测到目标时的回调，更新黑板的 TargetActor 值 */
	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

protected:
	/** 当前黑板组件引用，缓存自行为树运行后获取的黑板 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBlackboardComponent> BBComponent;

	/** 视觉感知配置：视野半径、角度、记忆时长等参数 */
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
};