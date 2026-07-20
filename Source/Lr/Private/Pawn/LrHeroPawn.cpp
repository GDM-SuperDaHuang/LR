// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrHeroPawn.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "NiagaraComponent.h"
#include "Actor/Weapon/LrWeaponBase.h"
#include "ASC/LrASC.h"
#include "Camera/CameraComponent.h"
#include "Component/Combat/LrPlayerCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/LrGAListDA.h"
#include "GameFramework/SpringArmComponent.h"
#include "Lib/LrCommonLibrary.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Air/LrAirMovementMode.h"
#include "Mover/Nav/LrNavMovementComponent.h"
#include "Perception/AISense_Sight.h"
#include "Player/PS/LrPS.h"

ALrHeroPawn::ALrHeroPawn()
{
	LrCombatComponent = CreateDefaultSubobject<ULrPlayerCombatComponent>(TEXT("LrCombatComponent"));
	PrimaryActorTick.bCanEverTick = true;
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
	// StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	// StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
	// StimuliSource->RegisterWithPerceptionSystem();

	// =========================
	// 骨骼 →碰撞体
	// =========================
	// LrSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LrMesh"));
	LrSkeletalMeshComponent->SetupAttachment(LrCapsuleComponent);
	// LrSkeletalMeshComponent->SetRelativeLocation(FVector(0, 0, -88));
	// LrSkeletalMeshComponent->SetOnlyOwnerSee(false);
	// LrSkeletalMeshComponent->SetOwnerNoSee(false);

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

	// =========================
	// 选中提示相关
	// =========================
	SelectionRing->SetupAttachment(RootComponent);
	
}

void ALrHeroPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetMPCParameter();
}

void ALrHeroPawn::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Controller = %s"), *GetNameSafe(GetController()));
}

void ALrHeroPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ALrHeroPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitASC();
	// 初始技能
	// FPawnTypeGAConfig LrDAConfig = ULrCommonLibrary::FindPawnTypeGAConfig(this, PawnType);
	// LrASC->AddGA(LrDAConfig.GATagList);
	LrASC->AddAllGA(this);
}

void ALrHeroPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitASC();
}

void ALrHeroPawn::InitASC()
{
	ALrPS* LrPS = GetPlayerState<ALrPS>();
	check(LrPS);
	// 绑定 自身与ps 到ASC
	LrPS->GetAbilitySystemComponent()->InitAbilityActorInfo(LrPS, this);
	LrASC = Cast<ULrASC>(LrPS->GetAbilitySystemComponent());
	LrAS = LrPS->GetAttributeSet();

	//ASC 初始化成功委托
	OnASCRegistered.Broadcast(LrASC);
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

void ALrHeroPawn::ToDie(const FLrDieParameters& LrDieConfig)
{
	Super::ToDie(LrDieConfig);
}
