// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrHeroPawn.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "MotionWarpingComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Actor/Weapon/LrWeaponBase.h"
#include "ASC/LrASC.h"
#include "ASC/AS/LrAS.h"
#include "Camera/CameraComponent.h"
#include "Component/Combat/LrPlayerCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/LrGAListDA.h"
#include "GameFramework/SpringArmComponent.h"
#include "Lib/LrCommonLibrary.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/LrAllModes.h"
#include "Mover/Air/LrAirMovementMode.h"
#include "Mover/Blink/LrBlinkMovementMode.h"
#include "Mover/Blink/LrKnockbackMovementMode.h"
#include "Mover/Death/LrDeathMovementMode.h"
#include "Mover/Nav/LrNavMovementComponent.h"
#include "Mover/Walk/LrWalkMovementMode.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Player/PS/LrPS.h"

ALrHeroPawn::ALrHeroPawn()
{
	LrCombatComponent = CreateDefaultSubobject<ULrPlayerCombatComponent>(TEXT("LrCombatComponent"));
	PrimaryActorTick.bCanEverTick = false;
	LrCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	RootComponent = LrCapsuleComponent;
	LrCapsuleComponent->InitCapsuleSize(34.f, 88.f);
	LrCapsuleComponent->SetCollisionProfileName(TEXT("LrHeroPawnCapsuleComponent"));


	// 相机 弹簧臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 800.f;

	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>("TopDownCameraComponent");
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	// =========================
	// 感知组件
	// =========================
	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
	StimuliSource->RegisterWithPerceptionSystem();

	// =========================
	// 骨骼 →碰撞体
	// =========================
	LrSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LrMesh"));
	LrSkeletalMeshComponent->SetupAttachment(LrCapsuleComponent);
	LrSkeletalMeshComponent->SetRelativeLocation(FVector(0, 0, -88));
	LrSkeletalMeshComponent->SetOnlyOwnerSee(false);
	LrSkeletalMeshComponent->SetOwnerNoSee(false);

	// =========================
	// 武器 →骨架
	// =========================
	EquippedWeaponComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("EquippedWeapon"));
	EquippedWeaponComponent->SetupAttachment(LrSkeletalMeshComponent); // 或 RootComponent，根据需求
	// EquippedWeaponComponent->SetChildActorClass(ALrWeaponBase::StaticClass()); // 默认类，可后期覆盖


	// =========================
	// NS→武器
	// =========================

	
	// =========================
	// 运动扭曲 
	// =========================
	LrMotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));


	// =========================
	// Mover
	// =========================
	LrMoverComponent->SetUpdatedComponent(LrCapsuleComponent);
	// CharacterMotionComponent->SetIsReplicated(true);


	// =========================
	// Nav → Mover 桥接
	// =========================
	LrNavMoverComponent = CreateDefaultSubobject<ULrNavMovementComponent>(TEXT("NavMoverComponent"));
	LrNavMoverComponent->UpdatedComponent = LrCapsuleComponent;

	/**
	 * true：这个组件，是否会被 NavigationSystem 考虑为：
	 * 1，动态障碍
	 * 2，NavMesh 生成 / 更新的参与者
	 */
	LrCapsuleComponent->SetCanEverAffectNavigation(false);
	// CharacterMotionComponent->StartingMovementMode = LrAllModes::Walking;
}

void ALrHeroPawn::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Controller = %s"), *GetNameSafe(GetController()));

	// if (LrMoverComponent)
	// {
	// 	// 添加特定的自定义移动模式
	// 	LrMoverComponent->AddMovementModeFromClass(TEXT("LrWalk"), ULrWalkMovementMode::StaticClass());
	// }

	if (LrMoverComponent)
	{
		//  清空旧模式（防止重复注册）
		LrMoverComponent->MovementModes.Empty();

		// 创建新的移动模式对象：行走模式、空中模式
		// 使用 RealisticMovementDefines 中定义的键名（例如 RealisticModes::Walk
		LrMoverComponent->MovementModes.Add(LrAllModes::Walk, NewObject<ULrWalkMovementMode>(LrMoverComponent));
		LrMoverComponent->MovementModes.Add(LrAllModes::Air, NewObject<ULrAirMovementMode>(LrMoverComponent));
		LrMoverComponent->MovementModes.Add(LrAllModes::Blink, NewObject<ULrBlinkMovementMode>(LrMoverComponent));
		LrMoverComponent->MovementModes.Add(LrAllModes::Knock, NewObject<ULrKnockbackMovementMode>(LrMoverComponent));
		LrMoverComponent->MovementModes.Add(LrAllModes::Death, NewObject<ULrDeathMovementMode>(LrMoverComponent));

		// 清空显式的状态转换表（转换逻辑已内置于各移动模式内部）
		// Удаляем явные переходы (Transitions), они теперь встроенны в логику режимов
		LrMoverComponent->Transitions.Empty();

		// 设置起始模式为空中模式（防止角色一开始就卡在地面下）
		// 通常空中模式会在落地时自动切换到行走模式
		// Устанавливаем стартовый режим
		LrMoverComponent->QueueNextMode(LrAllModes::Air);
	}
}

void ALrHeroPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	// if (LrMoverComponent)
	// {
	// 	// 添加特定的自定义移动模式
	// 	LrMoverComponent->AddMovementModeFromClass(TEXT("LrWalk"), ULrWalkMovementMode::StaticClass());
	//
	// 	// 默认激活模式设定视需求而定
	// 	// ⭐ 核心：设置初始模式名字
	// 	LrMoverComponent->StartingMovementMode = TEXT("LrWalk");
	// }
}

void ALrHeroPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ALrPS* LrPS = GetPlayerState<ALrPS>();
	check(LrPS);
	// 绑定 自身与ps 到ASC
	LrPS->GetAbilitySystemComponent()->InitAbilityActorInfo(LrPS, this);

	LrASC = Cast<ULrASC>(LrPS->GetAbilitySystemComponent());
	//ASC 初始化成功委托
	OnASCRegistered.Broadcast(LrASC);
	LrAS = LrPS->GetAttributeSet();
	// 初始技能
	ULrASC* ASC = CastChecked<ULrASC>(LrASC);
	FPawnTypeGAConfig LrDAConfig = ULrCommonLibrary::FindPawnTypeGAConfig(this, PawnType);
	ASC->AddGA(LrDAConfig.GATagList);
}

void ALrHeroPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	ALrPS* LrPS = GetPlayerState<ALrPS>();
	check(LrPS);
	// 绑定 自身与ps 到ASC
	LrPS->GetAbilitySystemComponent()->InitAbilityActorInfo(LrPS, this);
	LrASC = Cast<ULrASC>(LrPS->GetAbilitySystemComponent());
}

ULrAS* ALrHeroPawn::GetAS() const
{
	ALrPS* LrPS = GetPlayerState<ALrPS>();
	return LrPS->GetAttributeSet();
}

void ALrHeroPawn::EquipWeapon(FLrWeaponConfig WeaponConfig)
{
	if (!HasAuthority()) return; // 服务器执行

	// 先卸下旧武器
	if (ALrWeaponBase* OldWeapon = Cast<ALrWeaponBase>(EquippedWeaponComponent->GetChildActor()))
	{
		if (OldWeapon) OldWeapon->OnUnequipped();
	}

	// 设置新武器
	EquippedWeaponComponent->SetChildActorClass(WeaponConfig.WeaponClass);
	// EquippedWeaponComponent->SetChildActorClass(ALrWeaponBase::StaticClass()); // 默认类，可后期覆盖

	if (ALrWeaponBase* NewWeapon = Cast<ALrWeaponBase>(EquippedWeaponComponent->GetChildActor()))
	{
		NewWeapon->OnEquipped(this, WeaponConfig);
	}
	// 通知客户端（如果需要）
	// MarkPackageDirty() 或用 RPC
}

void ALrHeroPawn::Unequipped(FLrWeaponConfig WeaponConfig)
{
	if (!HasAuthority()) return; // 服务器执行

	// 先卸下旧武器
	if (ALrWeaponBase* OldWeapon = Cast<ALrWeaponBase>(EquippedWeaponComponent->GetChildActor()))
	{
		if (OldWeapon) OldWeapon->OnUnequipped();
	}
}
