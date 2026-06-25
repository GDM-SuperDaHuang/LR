// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Combat/LrAICombatComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/World.h"
#include "Lib/LrCommonLibrary.h"
#include "Pawn/LrEnemyPawn.h"

// Sets default values for this component's properties
ULrAICombatComponent::ULrAICombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TWeakObjectPtr<AActor> ULrAICombatComponent::GetClosestEnemyInCone(const FLrCombatQueryParams& Params)
{
	return CachedTargetActor.Get();
}

bool ULrAICombatComponent::CanAttack(AActor* Target, const float AttackRange, const float AttackCooldown)
{
	if (!Target)
	{
		return false;
	}

	// CD 检查
	const float Time = GetWorld()->GetTimeSeconds();
	if (Time - LastAttackTime < AttackCooldown)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}
	//----------------------------------------
	// 距离检查
	//----------------------------------------
	const FVector SelfLocation = Owner->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	const float Distance = FVector::Distance(SelfLocation, TargetLocation);
	if (Distance > AttackRange)
	{
		return false;
	}
	AActor* TargetActor = Cast<AActor>(Target);
	CachedTargetActor = TargetActor;
	return true;
}

bool ULrAICombatComponent::StartAttack()
{
	const ALrEnemyPawn* Enemy = Cast<ALrEnemyPawn>(GetOwner());
	if (!Enemy)
	{
		return false;
	}

	// 随机选择一个技能
	ALrGameModeBase* LrGameModeBase = ULrCommonLibrary::GetLrGameModeBase(Enemy);
	if (!LrGameModeBase)
	{
		return false;
	}
	FGameplayTag GATag;
	if (LrGameModeBase->LrGAListDA)
	{
		for (FLrPawnTypeGAConfig LrAllPawnTypeGaConfig : LrGameModeBase->LrGAListDA->LrAllPawnTypeGAConfig)
		{
			if (LrAllPawnTypeGaConfig.PawnType == Enemy->PawnType)
			{
				int32 Length = LrAllPawnTypeGaConfig.AllLrGAConfig.Num();
				int32 RandomIndex = FMath::RandRange(0, Length - 1);
				GATag = LrAllPawnTypeGaConfig.AllLrGAConfig[RandomIndex].GATag;
			}
		}
	}

	// 通过 GAS 蓝图库获取 Pawn 上的 AbilitySystemComponent
	// 不直接 Cast 是因为 ASC 可能在 PlayerState 或 Pawn 上，蓝图库会统一查找
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC)
	{
		return false;
	}
	// 尝试激活所有匹配 AbilityTag 的技能
	// 如果有多个技能共享同一标签，它们都会被尝试激活
	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(GATag));
	if (!bActivated)
	{
		return false;
	}

	LastAttackTime = GetWorld()->GetTimeSeconds();
	return true;
}

void ULrAICombatComponent::FinishAttack() const
{
	OnAttackFinished.Broadcast();
}
