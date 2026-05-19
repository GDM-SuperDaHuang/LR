// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrEnemyPawn.h"

#include "ASC/LrASC.h"
#include "ASC/AS/LrAS.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "MotionWarpingComponent.h"
#include "AI/Controller/LrAIControllerBase.h"
#include "AI/Subsystem/LrAIManagerSubsystem.h"
#include "Component/LrAIStateComponent.h"
#include "Component/LrPatrolRouteComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Air/LrAirMovementMode.h"
#include "Mover/Nav/LrNavMovementComponent.h"
#include "Mover/Walk/LrWalkMovementMode.h"

ALrEnemyPawn::ALrEnemyPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	// =========================
	// AI
	// =========================
	AIStateComponent =CreateDefaultSubobject<ULrAIStateComponent>(TEXT("AIStateComponent"));
	PatrolRoute =CreateDefaultSubobject<ULrPatrolRouteComponent>(TEXT("PatrolRoute"));
	
	// =========================
	// 骨骼 →碰撞体
	// =========================
	LrCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("EnemyCapsule"));
	RootComponent = LrCapsuleComponent;
	LrCapsuleComponent->InitCapsuleSize(34.f, 88.f);
	LrCapsuleComponent->SetCollisionProfileName(TEXT("LrEnemyPawnCapsuleComponent"));
	LrCapsuleComponent->SetCanEverAffectNavigation(true);

	LrSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LrEnemyMesh"));
	LrSkeletalMeshComponent->SetupAttachment(LrCapsuleComponent);
	LrSkeletalMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	LrSkeletalMeshComponent->SetOnlyOwnerSee(false);
	LrSkeletalMeshComponent->SetOwnerNoSee(false);
	
	// =========================
	// 武器 →骨架
	// =========================
	EquippedWeaponComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("EnemyEquippedWeapon"));
	EquippedWeaponComponent->SetupAttachment(LrSkeletalMeshComponent);

	// =========================
	// 运动扭曲 
	// =========================
	LrMotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("EnemyMotionWarping"));

	// =========================
	// Mover
	// =========================
	CharacterMotionComponent->SetUpdatedComponent(LrCapsuleComponent);

	// =========================
	// Nav → Mover 桥接
	// =========================
	LrNavMoverComponent = CreateDefaultSubobject<ULrNavMovementComponent>(TEXT("EnemyNavMoverComponent"));
	LrNavMoverComponent->SetUpdatedComponent(LrCapsuleComponent);
                   
	LrASC = CreateDefaultSubobject<ULrASC>(TEXT("EnemyASC"));
	LrASC->SetIsReplicated(true);
	LrASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	LrAS = CreateDefaultSubobject<ULrAS>(TEXT("EnemyAS"));

	// =========================
	// UI
	// =========================
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyHPBarWidget"));
	HealthBarWidget->SetupAttachment(LrCapsuleComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	
	bUseControllerRotationYaw = false;
}

void ALrEnemyPawn::BeginPlay()
{
	Super::BeginPlay();
	HomeLocation = GetActorLocation();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ULrAIManagerSubsystem* AISubsystem =World->GetSubsystem<ULrAIManagerSubsystem>();
	if (!AISubsystem)
	{
		return;
	}

	AISubsystem->SpawnNormalAI(this);

	if (CharacterMotionComponent)
	{
		//  清空旧模式（防止重复注册）
		CharacterMotionComponent->MovementModes.Empty();

		// 创建新的移动模式对象：行走模式、空中模式
		// 使用 RealisticMovementDefines 中定义的键名（例如 RealisticModes::Walk
		CharacterMotionComponent->MovementModes.Add(LrAllModes::Walk, NewObject<ULrWalkMovementMode>(CharacterMotionComponent));
		CharacterMotionComponent->MovementModes.Add(LrAllModes::Air, NewObject<ULrAirMovementMode>(CharacterMotionComponent));

		// 清空显式的状态转换表（转换逻辑已内置于各移动模式内部）
		// Удаляем явные переходы (Transitions), они теперь встроенны в логику режимов
		CharacterMotionComponent->Transitions.Empty();

		// 设置起始模式为空中模式（防止角色一开始就卡在地面下）
		// 通常空中模式会在落地时自动切换到行走模式
		// Устанавливаем стартовый режим
		CharacterMotionComponent->QueueNextMode(LrAllModes::Air);
	}

	if (GetMovementComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Movement=%s]"), *GetMovementComponent()->GetName());
	}
}

// UPawnMovementComponent* ALrEnemyPawn::GetMovementComponent() const
// {
// 	TCopyQualifiersFromTo_T<ULrNavMovementComponent, UPawnMovementComponent>* CopyQualifiersFromTo = Cast<UPawnMovementComponent>(LrNavMoverComponent);
// 	if (!CopyQualifiersFromTo)
// 	{
// 		return nullptr;
// 	}
// 	return CopyQualifiersFromTo;
// }

// FVector ALrEnemyPawn::GetNavAgentLocation() const
// {
// 	if (LrCapsuleComponent)
// 	{
// 		return LrCapsuleComponent->GetComponentLocation();
// 	}
//
// 	return GetActorLocation();
// }

void ALrEnemyPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	LrASC->InitAbilityActorInfo(this, this);
	OnASCRegistered.Broadcast(LrASC);
	if (StartupAbilities.Num() > 0)
	{
		ULrASC* LrEnemyASC = CastChecked<ULrASC>(LrASC);
		LrEnemyASC->AddGA(StartupAbilities);
	}
}

FVector ALrEnemyPawn::GetHomeLocation() const
{
	return HomeLocation;
}
