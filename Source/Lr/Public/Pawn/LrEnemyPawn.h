// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/LrCorpseConfigDA.h"
#include "Pawn/LrPawnBase.h"
#include "LrEnemyPawn.generated.h"

class UAnimMontage;
class ULrSTComponent;
class ULrWorldBarWidget;
class ULrWorldWidgetComponent;
class ULrBarWidget;
class ULrAIStateComponent;
class ULrPatrolRouteComponent;
class UCapsuleComponent;
class ULrASC;
class ULrAS;
class UWidgetComponent;
class UBehaviorTree;

class ALrEnemyPawn;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnemyDeath, ALrEnemyPawn*);

// DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float);

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

	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;
	virtual uint8 GetClassID() const override;
	//生成尸体
	void SpawnCorpse(const FLrDieParameters& DieParam, FLrCorpseConfig CorpseConfig);


	virtual void ToDie(const FLrDieParameters& DieParam) override;
	/** 面向相关 */
	//面向移动方向
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RotationInterpSpeed = 10.f;
	void FaceToDirection(const FVector& Direction, float DeltaTime);
	//面向目标
	void FaceToTarget(const FVector& TargetLocation, float DeltaTime);
	/** 面向相关 */

	/** AI相关 */
	FORCEINLINE
	ULrAIStateComponent* GetAIStateComponent() const
	{
		// return AIStateComponent;
		return nullptr;
	}

	FORCEINLINE
	ULrPatrolRouteComponent* GetPatrolRoute() const
	{
		return PatrolRoute;
	}

	FORCEINLINE
	ULrWorldWidgetComponent* GetWorldWidgetComponent() const
	{
		return LrWidgetComponent;
	}

	FVector GetHomeLocation() const;
	void PlayDeathMontage(UAnimMontage* Montage);
	void FinishDeath();

	/** 获取该敌人配置的行为树资产 */
	// FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
	/** AI相关 End */


	/** 敌人死亡事件，广播给 AI 控制器和其他监听者 */
	FOnEnemyDeath OnEnemyDeath;

protected:
	// UPROPERTY(VisibleAnywhere)
	// TObjectPtr<ULrAIStateComponent> AIStateComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULrPatrolRouteComponent> PatrolRoute;


	UPROPERTY()
	FVector HomeLocation;

	/** 碰撞体，作为 AI 导航代理和 Mover 系统的根依赖 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PawnInfo")
	TObjectPtr<UCapsuleComponent> LrCapsuleComponent;

	/** 世界空间血条 UI 组件，悬浮在敌人头顶,飘字等 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ULrWorldWidgetComponent> LrWidgetComponent;
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<ULrWorldBarWidget> LrWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<ULrSTComponent> LrStateTreeComponent;
};
