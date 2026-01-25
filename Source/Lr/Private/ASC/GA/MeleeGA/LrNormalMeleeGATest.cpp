// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GA/MeleeGA/LrNormalMeleeGATest.h"

#include "AbilitySystemComponent.h"
#include "MotionWarpingComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Data/LrGAListDA.h"
#include "Lib/LrCommonLibrary.h"
#include "Pawn/LrPawnBase.h"
#include "Tags/LrGameplayTags.h"

void ULrNormalMeleeGATest::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	FLrGAConfig LrDAConfig = ULrCommonLibrary::FindGAByTag(OwnerPawn, LrGameplayTags.GA_2);

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

	UAbilitySystemComponent* ASC = ULrCommonLibrary::GetASC(OwnerPawn);
	if (ASC)
	{
		ASC->AddLooseGameplayTag(LrGameplayTags.State_Block_Move);
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

	PlayTask->OnCompleted.AddDynamic(this, &ULrNormalMeleeGATest::OnMontageFinished);
	PlayTask->OnInterrupted.AddDynamic(this, &ULrNormalMeleeGATest::OnMontageFinished);
	PlayTask->OnCancelled.AddDynamic(this, &ULrNormalMeleeGATest::OnMontageFinished);
	PlayTask->ReadyForActivation();


	// ========== 4. 等待攻击真正生效通知 ==========
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		LrGameplayTags.Montage_Event_Attack_GA2
	);


	EventTask->EventReceived.AddDynamic(
		this,
		&ULrNormalMeleeGATest::OnAttackEvent
	);

	EventTask->ReadyForActivation();

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this, ASC,LrGameplayTags,Handle, ActorInfo, ActivationInfo]()
		{
			if (IsActive())
			{
				// EndAbility 时
				ASC->RemoveLooseGameplayTag(LrGameplayTags.State_Block_Move);
				EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			}
		},
		3.0f, // Montage 最大时间 + buffer
		false
	);
}

void ULrNormalMeleeGATest::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void ULrNormalMeleeGATest::OnMontageFinished()
{
	FLrGameplayTags LrGameplayTags = FLrGameplayTags::Get();
	UAbilitySystemComponent* ASC = ULrCommonLibrary::GetASC(GetActorInfo().AvatarActor.Get());
	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(LrGameplayTags.State_Block_Move);
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void ULrNormalMeleeGATest::OnAttackEvent(FGameplayEventData Payload)
{
	ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(GetAvatarActorFromActorInfo());
	if (!OwnerPawn) return;

	UAbilitySystemComponent* ASC = OwnerPawn->GetAbilitySystemComponent();
	if (!ASC) return;

	// ================== 1. 武器轨迹（所有客户端） ==================
	FGameplayCueParameters CueParams;
	CueParams.SourceObject = OwnerPawn;
	CueParams.Location = OwnerPawn->GetActorLocation();

	ASC->ExecuteGameplayCue(
		FLrGameplayTags::Get().GameplayCue_Melee_Trail,
		CueParams
	);
	
	// ========== 1. 播放武器轨迹 Niagara ==========
	SpawnWeaponTrailFX(OwnerPawn);

	// ========== 2. 命中检测 ==========
	TArray<FHitResult> HitResults;
	PerformMeleeTrace(OwnerPawn, HitResults);

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == OwnerPawn) continue;
		// ================== 3. 命中特效 ==================
		FGameplayCueParameters HitCueParams;
		HitCueParams.Location = Hit.ImpactPoint;
		HitCueParams.Normal = Hit.ImpactNormal;
		HitCueParams.TargetAttachComponent = Hit.GetComponent();
		ASC->ExecuteGameplayCue(
			FLrGameplayTags::Get().GameplayCue_Melee_Hit,
			HitCueParams
		);
		// ========== 3. 应用伤害 ==========
		// ApplyDamageToTarget(HitActor, Hit);
		//
		// // ========== 4. 敌人血液特效 ==========
		// PlayHitBloodFX(HitActor, Hit);
	}
}

void ULrNormalMeleeGATest::SpawnWeaponTrailFX(ALrPawnBase* OwnerPawn)
{
	
}

void ULrNormalMeleeGATest::PerformMeleeTrace(ALrPawnBase* OwnerPawn, TArray<FHitResult>& Array)
{
}

void ULrNormalMeleeGATest::OnAttackEventReceived(const FGameplayEventData* GameplayEventData) const
{
}
