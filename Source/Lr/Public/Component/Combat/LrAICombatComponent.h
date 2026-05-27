// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LrCombatComponentBase.h"
#include "LrAICombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackFinished);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrAICombatComponent : public ULrCombatComponentBase
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULrAICombatComponent();
	virtual AActor* GetClosestEnemyInCone(const FLrCombatQueryParams& Params) override;

	// 单体类型
	UPROPERTY()
	TWeakObjectPtr<AActor> CachedTargetActor; //目标可能死亡
	// 是否能攻击
	bool CanAttack(AActor* Target);
	// 开始攻击
	bool StartAttack();

	// 攻击结束 是否需要??
	void FinishAttack() const;

	UPROPERTY(BlueprintAssignable)
	FOnAttackFinished OnAttackFinished;

protected:
	// Called when the game starts
	// virtual void BeginPlay() override;

	// 攻击距离
	UPROPERTY(EditDefaultsOnly)
	float AttackRange = 250.f;

	// 攻击 CD
	UPROPERTY(EditDefaultsOnly)
	float AttackCooldown = 1.5f;

	// 上次攻击时间
	float LastAttackTime = -100.f;

	int32 ComboIndex = 0;
	int32 MaxCombo = 3;

	bool bInComboWindow = false;
};
