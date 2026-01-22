// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrHeroPawn.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "ASC/LrASC.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "DefaultMovementSet/Modes/SmoothWalkingMode.h"
#include "GameFramework/SpringArmComponent.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Nav/LrNavMovementComponent.h"
#include "Mover/Walk/LrWalkMovementMode.h"
#include "Player/PS/LrPS.h"

ALrHeroPawn::ALrHeroPawn()
{
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
	WeaponSKM = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LrMesh"));
	// 挂到右手插槽，可在子类改 Socket 名，注意名称 WeaponHandSocket 一定要一致
	WeaponSKM->SetupAttachment(LrSkeletalMeshComponent, FName(TEXT("WeaponSocket1")));
	// 武器本身不产生物理碰撞
	WeaponSKM->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	
	// =========================
	// Mover
	// =========================
	// LrMoverComponent = CreateDefaultSubobject<ULrMoverComponent>(TEXT("MoverComponent"));
	// LrMoverComponent->SetUpdatedComponent(LrCapsuleComponent);

	CharacterMotionComponent = CreateDefaultSubobject<UCharacterMoverComponent>(TEXT("MoverComponent"));
	CharacterMotionComponent->SetUpdatedComponent(LrCapsuleComponent);
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
	LrCapsuleComponent->SetCanEverAffectNavigation(true);

	// =========================
	// AI 设置
	// =========================
	// AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	// AIControllerClass = AAIController::StaticClass();
	// bUseControllerRotationYaw = false;

	// AutoPossessPlayer = EAutoReceiveInput::Player0; // ⭐关键


	// 添加特定的自定义移动模式
	// LrMoverComponent->AddMovementModeFromClass(TEXT("LrWalk"), ULrWalkMovementMode::StaticClass());
	// 默认激活模式设定视需求而定
	// ⭐ 核心：设置初始模式名字
	// LrMoverComponent->StartingMovementMode = TEXT("LrWalk");
	CharacterMotionComponent->StartingMovementMode = TEXT("LrWalk");
	// CharacterMotionComponent->StartingMovementMode = DefaultModeNames::Walking;


	
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
	CharacterMotionComponent->AddMovementModeFromClass(TEXT("LrWalk"), USmoothWalkingMode::StaticClass());
	// if (CharacterMotionComponent)
	// {
	// 	// 添加特定的自定义移动模式
	// 	CharacterMotionComponent->AddMovementModeFromClass(TEXT("Walk"), UWalkingMode::StaticClass());
	// }
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
	// AttributeSet = LrAS->GetAttributeSet();
	// todo ???
	// OnASCRegistered.Broadcast(AbilitySystemComponent);
	LrASC = LrPS->GetAbilitySystemComponent();
	LrAS = LrPS->GetAttributeSet();
	// 初始技能
	ULrASC* ASC = CastChecked<ULrASC>(LrASC);
	ASC->AddGA(InitGAListConfig);
}

void ALrHeroPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}
