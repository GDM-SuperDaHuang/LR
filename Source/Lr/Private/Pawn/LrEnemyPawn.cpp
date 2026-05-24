// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrEnemyPawn.h"

#include "ASC/LrASC.h"
#include "ASC/AS/LrAS.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"
#include "AI/Controller/LrAIControllerBase.h"
#include "AI/Subsystem/LrAIManagerSubsystem.h"
#include "Alembic/AbcGeom/IFaceSet.h"
#include "Component/LrAIStateComponent.h"
#include "Component/LrCombatComponent.h"
#include "Component/LrPatrolRouteComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Lib/LrCommonLibrary.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Air/LrAirMovementMode.h"
#include "Mover/Nav/LrNavMovementComponent.h"
#include "Mover/Walk/LrWalkMovementMode.h"
#include "UI/Component/LrWorldWidgetComponent.h"
#include "UI/Widget/Bar/WorldBar/LrWorldBarWidget.h"

ALrEnemyPawn::ALrEnemyPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	// =========================
	// AI
	// =========================
	AIStateComponent = CreateDefaultSubobject<ULrAIStateComponent>(TEXT("AIStateComponent"));
	LrCombatComponent = CreateDefaultSubobject<ULrCombatComponent>(TEXT("LrCombatComponent"));

	PatrolRoute = CreateDefaultSubobject<ULrPatrolRouteComponent>(TEXT("PatrolRoute"));
	AutoPossessAI = EAutoPossessAI::Disabled; //禁止自动Possess,自己直接调用
	// =========================
	// 骨骼 →碰撞体
	// =========================
	LrCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("EnemyCapsule"));
	RootComponent = LrCapsuleComponent;
	LrCapsuleComponent->InitCapsuleSize(34.f, 88.f);
	LrCapsuleComponent->SetCollisionProfileName(TEXT("LrEnemyPawnCapsuleComponent"));
	LrCapsuleComponent->SetCanEverAffectNavigation(false);

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

	bUseControllerRotationYaw = false;

	// =========================
	// 敌人UI
	// =========================
	HealthBarComponent = CreateDefaultSubobject<ULrWorldWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetRelativeLocation(FVector(0, 0, 120.f));
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

	ULrAIManagerSubsystem* AISubsystem = World->GetSubsystem<ULrAIManagerSubsystem>();
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

	if (HealthBarWidgetClass)
	{
		HealthBarComponent->SetWidgetClass(HealthBarWidgetClass);
	}

	HealthBarComponent->InitWidget();

	ULrWorldBarWidget* BarWidget = Cast<ULrWorldBarWidget>(HealthBarComponent->GetUserWidgetObject());

	if (BarWidget)
	{
		BarWidget->InitWidget(this);
		//TODO
		BarWidget->UpdateHealth(500);
	}

	OnHealthChanged.AddLambda(
		[BarWidget](float NewPercent)
		{
			if (BarWidget)
			{
				BarWidget->UpdateHealth(NewPercent);
			}
		});
}

void ALrEnemyPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	LrASC->InitAbilityActorInfo(this, this);
	OnASCRegistered.Broadcast(LrASC);
	FPawnTypeGAConfig LrDAConfig = ULrCommonLibrary::FindPawnTypeGAConfig(this, PawnType);
	LrASC->AddGA(LrDAConfig.GATagList);
}

void ALrEnemyPawn::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	// Super::GetActorEyesViewPoint(OutLocation, OutRotation);
	OutLocation = GetActorLocation() + FVector(0.f, 0.f, 60.f);
	OutRotation = GetActorRotation();
}

void ALrEnemyPawn::FaceToDirection(const FVector& Direction, float DeltaTime)
{
	if (Direction.IsNearlyZero())
	{
		return;
	}
	const FRotator Current = GetActorRotation();
	const FRotator Target = Direction.ToOrientationRotator();
	const FRotator NewRot = FMath::RInterpTo(Current, Target, DeltaTime, RotationInterpSpeed);
	SetActorRotation(NewRot);
}

void ALrEnemyPawn::FaceToTarget(const FVector& TargetLocation, float DeltaTime)
{
	FVector Direction = TargetLocation - GetActorLocation();
	Direction.Z = 0.f;
	FaceToDirection(Direction.GetSafeNormal(), DeltaTime);
}

FVector ALrEnemyPawn::GetHomeLocation() const
{
	return HomeLocation;
}
