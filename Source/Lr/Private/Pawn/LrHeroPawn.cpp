// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrHeroPawn.h"

#include "AIController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "DefaultMovementSet/Modes/SimpleWalkingMode.h"
#include "GameFramework/SpringArmComponent.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Nav/LrNavMovementComponent.h"
#include "Mover/Walk/LrWalkMovementMode.h"

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
	// Mover
	// =========================
	// LrMoverComponent = CreateDefaultSubobject<ULrMoverComponent>(TEXT("MoverComponent"));
	// LrMoverComponent->SetUpdatedComponent(LrCapsuleComponent);

	CharacterMotionComponent = CreateDefaultSubobject<UCharacterMoverComponent>(TEXT("MoverComponent"));
	CharacterMotionComponent->SetUpdatedComponent(LrCapsuleComponent);

	

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
	// CharacterMotionComponent->StartingMovementMode = TEXT("Walk");
	CharacterMotionComponent->StartingMovementMode = DefaultModeNames::Walking;
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
