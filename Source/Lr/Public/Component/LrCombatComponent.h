// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "LrCombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackFinished);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULrCombatComponent();

	// 是否正在攻击
	bool IsAttacking() const
	{
		return bAttacking;
	}
	// 是否能攻击
	bool CanAttack(AActor* Target) const;
	// 开始攻击
	bool StartAttack();
	// 攻击结束
	void FinishAttack();
	
	UPROPERTY(BlueprintAssignable)
	FOnAttackFinished OnAttackFinished;

protected:
	// Called when the game starts
	// virtual void BeginPlay() override;

	// 是否正在攻击
	UPROPERTY()
	bool bAttacking = false;

	// 攻击距离
	UPROPERTY(EditDefaultsOnly)
	float AttackRange = 250.f;

	// 朝向判定
	UPROPERTY(EditDefaultsOnly)
	float AttackDot = 0.7f;

	// 攻击 CD
	UPROPERTY(EditDefaultsOnly)
	float AttackCooldown = 1.5f;

	// 上次攻击时间
	float LastAttackTime = -100.f;

	int32 ComboIndex = 0;
	int32 MaxCombo = 3;

	bool bInComboWindow = false;
	

	
};
