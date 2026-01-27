// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GA/MeleeGA/LrNormalMeleeGA.h"

#include "AbilitySystemComponent.h"
#include "MotionWarpingComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Data/LrGAListDA.h"
#include "Kismet/KismetMathLibrary.h"
#include "Lib/LrCommonLibrary.h"
#include "Pawn/LrPawnBase.h"
#include "Tags/LrGameplayTags.h"


// ULrNormalMeleeGA::ULrNormalMeleeGA()
// {
// 	InputTag = FLrGameplayTags::Get().InputTag_J;
// }

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


	AActor* TargetActor = nullptr;
	// 如果没有目标，则使用前方作为默认方向
	FVector TargetLocation;
	if (TargetActor)
	{
		TargetLocation = TargetActor->GetActorLocation();
	}
	else
	{
		// 默认向前 500cm 作为“假目标点”
		TargetLocation = OwnerPawn->GetActorLocation() + OwnerPawn->GetActorForwardVector() * 500.f;
	}

	// ========== 2. 设置 Motion Warping ==========
	// 需要和蒙太奇运动扭曲轨道，
	OwnerPawn->LrMotionWarpingComponent->AddOrUpdateWarpTargetFromLocation("FacingTarget", TargetLocation);


	// ================== 2. 播放 Montage ==================
	FLrGameplayTags LrGameplayTags = FLrGameplayTags::Get();

	FLrGAConfig LrDAConfig = ULrCommonLibrary::FindGAByTag(OwnerPawn, LrGameplayTags.GA_1);

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
		LrGameplayTags.Montage_Event_Attack_GA1
	);


	EventTask->EventReceived.AddDynamic(
		this,
		&ULrNormalMeleeGA::OnAttackEvent
	);

	EventTask->ReadyForActivation();

	// ========== 7. 能力结束时机 ==========
	// 让蒙太age 的 AnimNotify 在播放到“收刀”帧时调用 EndAbility；
	// 如果 Notify 没触发，也设个兜底超时
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this, Handle, ActorInfo, ActivationInfo]()
		{
			if (IsActive())
			{
				EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			}
		},
		3.0f, // Montage 最大时间 + buffer
		false
	);
}

void ULrNormalMeleeGA::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void ULrNormalMeleeGA::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


void ULrNormalMeleeGA::OnAttackEventReceived(const FGameplayEventData* GameplayEventData) const
{
}


void ULrNormalMeleeGA::OnAttackEvent(FGameplayEventData Payload)
{
	ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(GetAvatarActorFromActorInfo());
	if (!OwnerPawn) return;

	// ========== 1. 播放武器轨迹 Niagara ==========
	SpawnWeaponTrailFX(OwnerPawn);

	// ========== 2. 命中检测 ==========
	TArray<FHitResult> HitResults;
	PerformMeleeTrace(OwnerPawn, HitResults);

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == OwnerPawn) continue;

		// ========== 3. 应用伤害 ==========
		// ApplyDamageToTarget(HitActor, Hit);
		//
		// // ========== 4. 敌人血液特效 ==========
		// PlayHitBloodFX(HitActor, Hit);
	}
}

void ULrNormalMeleeGA::SpawnWeaponTrailFX(ALrPawnBase* OwnerPawn)
{
}

void ULrNormalMeleeGA::PerformMeleeTrace(ALrPawnBase* OwnerPawn, TArray<FHitResult>& Array)
{
}
