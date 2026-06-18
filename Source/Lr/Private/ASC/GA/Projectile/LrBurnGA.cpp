// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GA/Projectile/LrBurnGA.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Actor/Projectile/LrProjectile.h"
#include "ASC/GA/MeleeGA/LrGAEnemyMeleeAttack.h"
#include "Data/LrGAListDA.h"
#include "Game/LrTickableWorldSubsystem.h"
#include "Lib/LrCommonLibrary.h"
#include "Mover/LrMoverComponent.h"
#include "Pawn/LrPawnBase.h"


void ULrBurnGA::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(ActorInfo->AvatarActor.Get());
	if (!OwnerPawn)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	//选择一个Montage
	FLrGAConfig LrDAConfig = ULrCommonLibrary::FindGAByTag(OwnerPawn, GetAssetTags().First());
	TArray<UAnimMontage*> AnimMontages = LrDAConfig.MontageList;
	int32 Length = AnimMontages.Num();
	if (Length <= 0)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	int32 RandomIndex = FMath::RandRange(0, Length - 1);
	UAnimMontage* MeleeMontage = AnimMontages[RandomIndex];

	if (ULrTickableWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULrTickableWorldSubsystem>())
	{
		Subsystem->SpawnProjectile(OwnerPawn,OwnerPawn->GetActorLocation(),);
		if ( Subsystem->SpawnProjectile())
		{
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(SocketLocation);
			SpawnTransform.SetRotation(Rotation.Quaternion());
			AcquireProjectile->FinishSpawning(SpawnTransform);
		}
	
	}
	// ========== 2. 设置 Motion Warping ==========
	// 需要和蒙太奇运动扭曲轨道，
	if (ULrCombatComponentBase* Combat = GetAvatarActorFromActorInfo()->FindComponentByClass<ULrCombatComponentBase>())
	{
		ConeParams.Origin = OwnerPawn->GetActorLocation();
		TargetAActor = Combat->GetClosestEnemyInCone(ConeParams);
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

	// 播放 Montage
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MeleeMontage,
		1.f);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Montage 正常结束
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	// Montage 被打断
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->ReadyForActivation();

	// ========== 4. 等待攻击真正生效通知 ==========
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		LrDAConfig.MontageEvent
	);

	EventTask->EventReceived.AddDynamic(this, &ULrGAEnemyMeleeAttack::OnAttackEvent);
	EventTask->ReadyForActivation();
}

void ULrBurnGA::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void ULrBurnGA::OnMontageFinished()
{
}
