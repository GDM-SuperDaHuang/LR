// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GA/Lightning/LrLightningGA.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Actor/Lightning/LrLightning.h"
#include "ASC/LrASC.h"
#include "Data/LrGAListDA.h"
#include "Engine/World.h"
#include "Game/LrTickableWorldSubsystem.h"
#include "Lib/LrCommonLibrary.h"
#include "Pawn/LrPawnBase.h"

ULrLightningGA::ULrLightningGA()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void ULrLightningGA::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("======ActivateAbility ULrLightningGA"));

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(ActorInfo->AvatarActor.Get());
	if (!OwnerPawn)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	//选择一个Montage
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

	// 播放 Montage
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MeleeMontage,
		1,
		NAME_None,
		true);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Montage 正常结束
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->ReadyForActivation();

	// 等待 Montage 中的伤害触发帧（如 AnimNotify 发送的 GameplayEvent）
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		LrDAConfig.MontageEvent);
	EventTask->EventReceived.AddDynamic(this, &ULrLightningGA::OnAttackEvent);
	EventTask->ReadyForActivation();


	// 松开时
	UAbilityTask_WaitInputRelease* WaitRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	WaitRelease->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
	WaitRelease->ReadyForActivation();
}

void ULrLightningGA::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


void ULrLightningGA::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	// 生成闪电 Actor
	if (ULrTickableWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULrTickableWorldSubsystem>())
	{
		AActor* OwnerActor = GetAvatarActorFromActorInfo();
		ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(OwnerActor);
		if (!OwnerPawn)
		{
			return;
		}
		ALrLightning* LrLightning = Subsystem->GetLightning(OwnerPawn);
		LrLightning->Stop();
	}
}

void ULrLightningGA::OnAttackEvent(FGameplayEventData Payload)
{
	AActor* OwnerActor = GetAvatarActorFromActorInfo();
	ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(OwnerActor);
	if (!OwnerPawn)
	{
		return;
	}

	// 生成闪电 Actor
	if (ULrTickableWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULrTickableWorldSubsystem>())
	{
		ALrLightning* LrLightning = Subsystem->GetLightning(OwnerPawn);
		LrLightning->Init(OwnerPawn);
	}
}

void ULrLightningGA::OnMontageFinished()
{
	// EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
