// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LrCombatComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Lib/LrCommonLibrary.h"
#include "Pawn/LrEnemyPawn.h"

// Sets default values for this component's properties
ULrCombatComponent::ULrCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool ULrCombatComponent::StartAttack()
{
	const ALrEnemyPawn* Enemy = Cast<ALrEnemyPawn>(GetOwner());
	if (!Enemy)
	{
		return false;
	}
	FPawnTypeGAConfig PawnTypeGaConfig = ULrCommonLibrary::FindPawnTypeGAConfig(Enemy, Enemy->PawnType);
	TArray<FGameplayTag> GameplayTags = PawnTypeGaConfig.GATagList;
	if (GameplayTags.Num() <= 0)
	{
		return true;
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
	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(GameplayTags[0]));
	if (!bActivated)
	{
		return false;
	}

	bAttacking = true;
	LastAttackTime = GetWorld()->GetTimeSeconds();
	return true;
}

void ULrCombatComponent::FinishAttack()
{
	bAttacking = false;
	OnAttackFinished.Broadcast();
}

bool ULrCombatComponent::CanAttack(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	// 正在攻击
	if (bAttacking)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// CD 检查
	const float Time = GetWorld()->GetTimeSeconds();
	if (Time - LastAttackTime < AttackCooldown)
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

	//----------------------------------------
	// 朝向检查
	//----------------------------------------
	const FVector Forward = Owner->GetActorForwardVector();
	const FVector Dir = (TargetLocation - SelfLocation).GetSafeNormal();
	const float Dot = FVector::DotProduct(Forward, Dir);

	// 0.7 ≈ 45°
	if (Dot < AttackDot)
	{
		return false;
	}
	return true;
}
