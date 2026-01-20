// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GA/MeleeGA/LrNormalMeleeGA.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Data/LrGAListDA.h"
#include "Kismet/KismetMathLibrary.h"
#include "Lib/LrCommonLibrary.h"
#include "Pawn/LrPawnBase.h"
#include "Tags/LrGameplayTags.h"


void ULrNormalMeleeGA::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


	// ========== 0. 合法性校验 ==========
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(ActorInfo->AvatarActor.Get());
	if (!OwnerPawn)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	FLrGameplayTags LrGameplayTags = FLrGameplayTags::Get();
	const FLrDAConfig* LrDAConfig = ULrCommonLibrary::FindGAByTag(OwnerPawn, LrGameplayTags.GA_1);
	if (LrDAConfig == nullptr)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}

	UAnimMontage* Montage = LrDAConfig->Montage;
	if (!Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. 监听 AnimNotify 事件
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		ASC->GenericGameplayEventCallbacks
		.FindOrAdd(LrGameplayTags.As_Attack)
		.AddUObject(this, &ULrNormalMeleeGA::OnAttackEventReceived);
	}

	// 3. 播放蒙太奇
	// GAS 官方推荐：AbilityTask_PlayMontageAndWait
	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		Montage,
		1.f
	);
	
	Task->OnCompleted.AddDynamic(this, &ULrNormalMeleeGA::OnMontageFinished);
	Task->OnInterrupted.AddDynamic(this, &ULrNormalMeleeGA::OnMontageFinished);
	Task->OnCancelled.AddDynamic(this, &ULrNormalMeleeGA::OnMontageFinished);
	Task->ReadyForActivation();

	
	//1, 获取 敌人目标
	// ========== 1. 获取敌人目标 ==========
	AActor* TargetActor = nullptr;
	if (ActorInfo->PlayerController != nullptr) // 玩家控制 → 取当前锁定敌人
	{
		// TargetActor = ULrCombatLibrary::GetLockedEnemy(OwnerCharacter);
	}
	else // AI 控制 → 取最近可攻击敌人
	{
		// TArray<AActor*> Enemies;
		// ULrCombatLibrary::GetNearbyEnemies(OwnerCharacter, 600.f, Enemies);
		// TargetActor = ULrCombatLibrary::SelectClosestTarget(OwnerCharacter, Enemies);
	}

	if (TargetActor == nullptr)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}


	
	//
	// 视觉方面
	//2,接受蒙太奇通知，然后面向敌人 todo
	OwnerPawn->SetActorRotation(UKismetMathLibrary::FindLookAtRotation(OwnerPawn->GetActorLocation(), TargetActor->GetActorLocation()));

	// 3，从数据资产读取，获取攻击蒙太奇(普通攻击有多个，随机挑选一个)，先播放。


	// 4，武器/攻击的 插槽位置

	// const FVector SocketLocation = OwnerPawn->LrSkeletalMeshComponent->GetSocketLocation( bRightHandSwing ? TEXT("Weapon_R") : TEXT("Weapon_L"));


	// 获取周围最近的敌人，如果是玩家则由玩家选择敌人


	// 造成伤害


	// 特效播放


	// ========== 7. 能力结束时机 ==========
	// 让蒙太age 的 AnimNotify 在播放到“收刀”帧时调用 EndAbility；
	// 如果 Notify 没触发，也设个兜底超时
	// FTimerHandle TimerHandle;
	// OwnerCharacter->GetWorldTimerManager().SetTimer(
	// 	TimerHandle, this, &ULrNormalMeleeGA::EndAbilityOnError,
	// 	MontageLength + 0.5f, false);
}

void ULrNormalMeleeGA::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void ULrNormalMeleeGA::OnMontageFinished()
{
	
}



void ULrNormalMeleeGA::OnAttackEventReceived(const FGameplayEventData* GameplayEventData) const
{
	
}


