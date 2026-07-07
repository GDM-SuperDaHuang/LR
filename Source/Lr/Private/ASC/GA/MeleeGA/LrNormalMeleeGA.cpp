// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GA/MeleeGA/LrNormalMeleeGA.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "MotionWarpingComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "ASC/LrASC.h"
#include "Component/Combat/LrPlayerCombatComponent.h"
#include "Data/LrGAListDA.h"
#include "Lib/LrCommonLibrary.h"
#include "Mover/LrMoverComponent.h"
#include "Pawn/LrPawnBase.h"


ULrNormalMeleeGA::ULrNormalMeleeGA()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
}

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

	// ========== 2. 设置 Motion Warping ==========
	if (ULrCombatComponentBase* Combat = GetAvatarActorFromActorInfo()->FindComponentByClass<ULrCombatComponentBase>())
	{
		ConeParams.Origin = OwnerPawn->GetActorLocation();
		ConeParams.Forward = OwnerPawn->GetActorForwardVector();
		TargetAActor = Combat->GetClosestEnemyInCone(ConeParams);
		// if (TargetAActor.Get())
		// {
		// 	OwnerPawn->LrMotionWarpingComponent->AddOrUpdateWarpTargetFromLocation("FacingTarget", TargetAActor->GetActorLocation());
		// }
		if (TargetAActor.Get())
		{
			FVector TargetLocation = TargetAActor->GetActorLocation();
			FVector OwnerLocation = OwnerPawn->GetActorLocation();

			// 计算从攻击者指向目标的方向旋转
			FRotator TargetRotation = (TargetLocation - OwnerLocation).Rotation();

			// 通常攻击转向只需要水平转身（Yaw），需要把 Pitch 和 Roll 清零，防止角色发生倾斜
			TargetRotation.Pitch = 0.f;
			TargetRotation.Roll = 0.f;
			OwnerPawn->LrMoverComponent->AttackWarpRotation = TargetRotation;

			UE_LOG(LogTemp, Warning, TEXT("OwnerYaw=%f TargetYaw=%f"),
			       OwnerPawn->GetActorRotation().Yaw,
			       TargetRotation.Yaw);
			// 使用带有 Rotation 的函数版本
			// OwnerPawn->LrMotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
			// 	FName("FacingTarget"),
			// 	TargetLocation,
			// 	TargetRotation
			// );
		}
	}

	// ================== 2. 播放 Montage ==================
	FLrGAConfig LrDAConfig = ULrCommonLibrary::FindGAConfig(OwnerPawn, GetAssetTags().First());
	TArray<UAnimMontage*> AnimMontages = LrDAConfig.MontageList;
	int32 Length = AnimMontages.Num();
	if (Length <= 0)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	int32 RandomIndex = FMath::RandRange(0, Length - 1);
	UAnimMontage* MeleeMontage = AnimMontages[RandomIndex];
	if (!MeleeMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	float MontagePlayRate = 1; //GetMontagePlayRate(); // 可根据需要调整速率
	FName SectionName = ""; //GetMontageSectionName();  // 可选：指定Section

	// float Length = GetAbilitySystemComponentFromActorInfo()->PlayMontage(
	// 		this,
	// 		CurrentSpecHandle,
	// 		CurrentActivationInfo,
	// 		MeleeMontage,
	// 		1.f
	// 	);

	// UE_LOG(LogTemp, Warning, TEXT("Montage Length: %f"), Length);
	// ========== 3. 播放近战 Montage ==========
	UAbilityTask_PlayMontageAndWait* PlayTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MeleeMontage,
		MontagePlayRate,
		NAME_None,
		true //结束了Montage 还在播（常见坑）
	);

	PlayTask->OnCompleted.AddDynamic(this, &ULrNormalMeleeGA::OnMontageFinished);
	PlayTask->OnInterrupted.AddDynamic(this, &ULrNormalMeleeGA::OnMontageFinished);
	PlayTask->OnCancelled.AddDynamic(this, &ULrNormalMeleeGA::OnMontageFinished);
	PlayTask->ReadyForActivation();


	// ========== 4. 等待攻击真正生效通知 ==========
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		LrDAConfig.MontageEvent
	);

	EventTask->EventReceived.AddDynamic(this, &ULrNormalMeleeGA::OnAttackEvent);
	EventTask->ReadyForActivation();
}

void ULrNormalMeleeGA::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void ULrNormalMeleeGA::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(GetAvatarActorFromActorInfo());
	if (!OwnerPawn) return;
	OwnerPawn->LrMoverComponent->bIsInAttackWarp = false;
	// OwnerPawn->LrMoverComponent->AttackWarpRotation = FRotator::ZeroRotator;
}


void ULrNormalMeleeGA::OnAttackEvent(FGameplayEventData Payload)
{
	// ========== 2. 应用伤害 ==========
	// 只在 Server 结算
	if (!GetAvatarActorFromActorInfo()->HasAuthority())
	{
		return;
	}
	ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(GetAvatarActorFromActorInfo());
	if (!OwnerPawn) return;

	// ========== 1. 播放武器轨迹 Niagara ==========
	SpawnWeaponTrailFX(OwnerPawn);


	if (ULrASC* LrASC = Cast<ULrASC>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		if (ULrCombatComponentBase* Combat = GetAvatarActorFromActorInfo()->FindComponentByClass<ULrCombatComponentBase>())
		{
			if (TargetAActor.Get())
			{
				DamageEffectParams.DamageValue = 5;
				LrASC->ApplyDamageToTarget(GetAvatarActorFromActorInfo(), TargetAActor.Get(), DamageEffectParams);
				OwnerPawn->LrMoverComponent->bIsInAttackWarp = true;
			}
		}
	}
}

void ULrNormalMeleeGA::SpawnWeaponTrailFX(ALrPawnBase* OwnerPawn)
{
}

void ULrNormalMeleeGA::PerformMeleeTrace(ALrPawnBase* OwnerPawn, TArray<FHitResult>& Array)
{
}
