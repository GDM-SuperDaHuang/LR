// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pawn/LrPawnBase.h"
#include "LrEnemyPawn.generated.h"

class UCapsuleComponent;
class ULrASC;
class ULrAS;
class UWidgetComponent;
class UBehaviorTree;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDeath, ALrEnemyPawn*, DeadEnemy);

/**
 * 敌方 Pawn 基类
 * 挂载碰撞体、Mover、导航桥接、ASC/AS、行为树和世界血条组件
 * 所有敌方角色蓝图均继承自此类型
 */
UCLASS()
class LR_API ALrEnemyPawn : public ALrPawnBase
{
	GENERATED_BODY()

public:
	ALrEnemyPawn();

	virtual void BeginPlay() override;

	/** AIController 接管时初始化 GAS 并授予出生技能 */
	virtual void PossessedBy(AController* NewController) override;

	/** 获取该敌人配置的行为树资产 */
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

	/** 获取敌人能力系统组件 */
	FORCEINLINE ULrASC* GetLrASC() const { return LrEnemyASC; }

	/** 敌人死亡事件，广播给 AI 控制器和其他监听者 */
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnEnemyDeath OnEnemyDeath;

protected:
	/** 碰撞体，作为 AI 导航代理和 Mover 系统的根依赖 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PawnInfo")
	TObjectPtr<UCapsuleComponent> LrCapsuleComponent;

	/** 能力系统组件，AI 敌人自行持有（不依赖 PlayerState） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<ULrASC> LrEnemyASC;

	/** 属性集，存储敌人的生命值、攻击力等数值 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<ULrAS> LrEnemyAS;

	/** AI 行为树资产，定义敌人的决策逻辑 */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	/** 出生时自动授予的 GAS 技能标签列表 */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<FGameplayTag> StartupAbilities;

	/** 世界空间血条 UI 组件，悬浮在敌人头顶 */
	UPROPERTY(VisibleAnywhere, Category = "UI")
	TObjectPtr<UWidgetComponent> HealthBarWidget;
};