// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GA/Projectile/LrBurnGA.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Actor/Projectile/LrProjectile.h"
#include "Component/Combat/LrAICombatComponent.h"
#include "Component/Combat/LrPlayerCombatComponent.h"
#include "Data/LrGAListDA.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
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

	EventTask->EventReceived.AddDynamic(this, &ULrBurnGA::OnAttackEvent);
	EventTask->ReadyForActivation();
}

void ULrBurnGA::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

/**
 * 
 * @param ProjectileTargetLocation 投射物最终要飞向的目标位置
 * @param bOverridePitch 是否强制覆盖导弹的俯仰角, 若为 true，则使用 PitchOverride 指定发射角度；若为 false，则根据 ProjectileTargetLocation 与发射位置自动计算角
 * @param PitchOverride 当 bOverridePitch 为 true 时，强制使用的俯仰角度值（单位：度）。例如设为 -10.0f 可使导弹略微向下发射。
 * @param HomingTarget 导弹要追踪的目标 Actor（如敌人角色）。若传入有效指针，导弹将开启追踪模式，持续飞向该 Actor 的中心位置；若为 nullptr，导弹将沿直线飞向 ProjectileTargetLocation。
 */
void ULrBurnGA::SpawnProjectiles(const FVector& ProjectileTargetLocation, bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
{
	// ===================== 第一步：获取施法者并验证权限 =====================
	// 从 Ability 系统中获取拥有此技能的 Avatar Actor（通常是角色/怪物）
	AActor* OwnerActor = GetAvatarActorFromActorInfo();
	if (!OwnerActor)
	{
		return;
	}
	bool bHasAuthority = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bHasAuthority) return;
	ILrCombatInterface* OwnerCombat = Cast<ILrCombatInterface>(OwnerActor);
	if (!OwnerCombat)
	{
		return;
	}

	// ===================== 第二步：计算发射方向和分散旋转 =====================
	// 根据目标位置与发射点计算出基础朝向
	// 从战斗接口获取发射点（通常是武器上的 Socket 位置）,如"WeaponTip"）获取发射位置
	FVector SocketLocation = OwnerCombat->GetProjectileLocation();

	// 1. 根据是否存在追踪目标，决定初始朝向瞄准谁
	FVector ActualTargetLocation = HomingTarget ? HomingTarget->GetActorLocation() : ProjectileTargetLocation; // 有追踪目标，直接瞄着目标当前的脚下, 无追踪目标，瞄着指定的世界坐标点
	FRotator Rotation = (ActualTargetLocation - SocketLocation).Rotation();
	if (bOverridePitch)
	{
		Rotation.Pitch = PitchOverride;
	}

	const FVector Forward = Rotation.Vector();
	// 此处 ProjectilesSpread 应为成员变量（定义在 GA 中），控制散射角度
	// EffectiveNumProjectiles 目前固定为 1，将来可扩展为多弹
	const int32 EffectiveNumProjectiles = 1;
	TArray<FRotator> FRotators = ULrCommonLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectilesSpread, EffectiveNumProjectiles);
	for (FRotator& Rot : FRotators)
	{
		//射击 ,并且在武器处产生
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rotation.Quaternion());
		// 生成一个Actor ,并且延迟初始化，不进行其他操作	
		ALrProjectile* Projectile = GetWorld()->SpawnActorDeferred<ALrProjectile>(
			ProjectileClass,
			SpawnTransform, // 初始 Transform
			GetOwningActorFromActorInfo(), // Owner（通常是施法角色）
			Cast<APawn>(GetOwningActorFromActorInfo()), // Instigator（负责计入伤害统计）
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn); // 碰撞策略：强行生成，不管是否卡墙

		// todo
		DamageEffectParams.DamageValue = 11;
		DamageEffectParams.Duration = 10.f;
		Projectile->DamageEffectParams = DamageEffectParams;
		Projectile->ProjectileOwner = OwnerActor;
		//追踪弹
		if (HomingTarget)
		{
			Projectile->Movement->HomingTargetComponent = HomingTarget->GetRootComponent();
			Projectile->Movement->HomingAccelerationMagnitude = 50.f; // 追踪加速度，值越大转向越迅猛

			FVector TargetLocation = HomingTarget->GetActorLocation();
			FVector OwnerLocation = OwnerActor->GetActorLocation();

			// 计算从攻击者指向目标的方向旋转
			FRotator TargetRotation = (TargetLocation - OwnerLocation).Rotation();
			ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(GetAvatarActorFromActorInfo());

			// 通常攻击转向只需要水平转身（Yaw），需要把 Pitch 和 Roll 清零，防止角色发生倾斜
			TargetRotation.Pitch = 0.f;
			TargetRotation.Roll = 0.f;
			OwnerPawn->LrMoverComponent->AttackWarpRotation = TargetRotation;
		}
		else
		{
			//注意指针！！！
			//这里的Projectile->ProjectileMovement->HomingTargetComponent是弱引用，如果目标的被销毁，那么该位置消失,Projectile->ProjectileMovement->HomingTargetComponent指向null
			// Projectile->ProjectileMovement->HomingTargetComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());

			//创建一个锚点，先缓存起来
			Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(Projectile);
			Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
			//设置目标地点
			Projectile->Movement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
		}
		// 是否启用追踪
		Projectile->Movement->bIsHomingProjectile = bLaunchHomingProjectiles;

		// 让导弹真正“启动” ，SpawnActorDeferred需要FinishSpawning， 完成投射物生成（应用之前的设置，触发AAuraProjectile的BeginPlay）
		/**
		 * FinishSpawning 会触发：
		 * AActor::PostActorCreated()
		 * UActorComponent::Activate()（含 ProjectileMovement）
		 * AActor::BeginPlay()
		 */
		Projectile->FinishSpawning(SpawnTransform);
	}
}

void ULrBurnGA::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(GetAvatarActorFromActorInfo());
	if (!OwnerPawn) return;
	OwnerPawn->LrMoverComponent->bIsInAttackWarp = false;
	// OwnerPawn->LrMoverComponent->AttackWarpRotation = FRotator::ZeroRotator;
}

void ULrBurnGA::OnAttackEvent(FGameplayEventData Payload)
{
	AController* InstigatorController = GetAvatarActorFromActorInfo()->GetInstigatorController();
	TWeakObjectPtr<AActor> TargetAActor;
	if (ULrAICombatComponent* AICombat = GetAvatarActorFromActorInfo()->FindComponentByClass<ULrAICombatComponent>())
	{
		ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(GetAvatarActorFromActorInfo());
		ConeParams.Origin = OwnerPawn->GetActorLocation();
		ConeParams.Forward = OwnerPawn->GetActorForwardVector();
		ConeParams.ConeLength = ConeLength;
		TargetAActor = AICombat->GetClosestEnemyInCone(ConeParams);
		if (TargetAActor.Get())
		{
			OwnerPawn->LrMoverComponent->bIsInAttackWarp = true;
		}
	}
	else if (ULrPlayerCombatComponent* PlayerCombat = GetAvatarActorFromActorInfo()->FindComponentByClass<ULrPlayerCombatComponent>())
	{
		ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(GetAvatarActorFromActorInfo());
		ConeParams.Origin = OwnerPawn->GetActorLocation();
		ConeParams.Forward = OwnerPawn->GetActorForwardVector();
		ConeParams.ConeLength = ConeLength;
		// TargetAActor = PlayerCombat->GetNearestPawnToCursor();
		TargetAActor = PlayerCombat->GetClosestEnemyInCone(ConeParams);
		//显示箭头
		if (ALrPawnBase* LrPawnBase = Cast<ALrPawnBase>(TargetAActor))
		{
			LrPawnBase->SetSelected(true);
			OwnerPawn->LrMoverComponent->bIsInAttackWarp = true;
		}
	}
	if (TargetAActor.IsValid())
	{
		SpawnProjectiles(TargetAActor.Get()->GetActorLocation(), false, 60, TargetAActor.Get());
	}
}
