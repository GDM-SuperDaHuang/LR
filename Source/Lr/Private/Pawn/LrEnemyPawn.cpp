// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrEnemyPawn.h"

#include "ASC/LrASC.h"
#include "ASC/AS/LrAS.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"
#include "AI/Controller/LrAIControllerBase.h"
#include "AI/ST/LrSTComponent.h"
#include "AI/Subsystem/LrAIManagerSubsystem.h"
#include "Alembic/AbcGeom/IFaceSet.h"
#include "Component/LrAIStateComponent.h"
#include "Component/LrPatrolRouteComponent.h"
#include "Component/Combat/LrAICombatComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Lib/LrCommonLibrary.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Air/LrAirMovementMode.h"
#include "Mover/Blink/LrBlinkMovementMode.h"
#include "Mover/Blink/LrKnockbackMovementMode.h"
#include "Mover/Death/LrDeathMovementMode.h"
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
	// AIStateComponent = CreateDefaultSubobject<ULrAIStateComponent>(TEXT("AIStateComponent"));
	LrCombatComponent = CreateDefaultSubobject<ULrAICombatComponent>(TEXT("LrCombatComponent"));

	PatrolRoute = CreateDefaultSubobject<ULrPatrolRouteComponent>(TEXT("PatrolRoute"));
	AutoPossessAI = EAutoPossessAI::Disabled; //禁止自动 Possess,自己直接调用
	// =========================
	// 骨骼 →碰撞体
	// =========================
	LrCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("EnemyCapsule"));
	RootComponent = LrCapsuleComponent;
	LrCapsuleComponent->InitCapsuleSize(34.f, 88.f);
	LrCapsuleComponent->SetCollisionProfileName(TEXT("LrEnemyPawnCapsuleComponent"));
	LrCapsuleComponent->SetCanEverAffectNavigation(false);
	LrCapsuleComponent->SetCollisionObjectType(ECC_Pawn);

	LrSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LrEnemyMesh"));
	LrSkeletalMeshComponent->SetupAttachment(LrCapsuleComponent);
	LrSkeletalMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	LrSkeletalMeshComponent->SetOnlyOwnerSee(false);
	LrSkeletalMeshComponent->SetOwnerNoSee(false);
	LrSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LrSkeletalMeshComponent->SetCanEverAffectNavigation(false);
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
	LrMoverComponent->SetUpdatedComponent(LrCapsuleComponent);

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
	LrWidgetComponent = CreateDefaultSubobject<ULrWorldWidgetComponent>(TEXT("HealthBarComponent"));
	LrWidgetComponent->SetupAttachment(RootComponent);
	LrWidgetComponent->SetRelativeLocation(FVector(0, 0, 120.f));

	// =========================
	// 状态树
	// =========================
	LrStateTreeComponent = CreateDefaultSubobject<ULrSTComponent>(TEXT("LrSTComponent"));

	
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
	
	if (LrWidgetClass)
	{
		LrWidgetComponent->SetWidgetClass(LrWidgetClass);
	}
	LrWidgetComponent->InitWidget();

	ULrWorldBarWidget* BarWidget = Cast<ULrWorldBarWidget>(LrWidgetComponent->GetUserWidgetObject());
	ULrAS* As = GetAS();

	if (BarWidget)
	{
		// BarWidget->InitWidget(this);
		for (TPair<FGameplayTag, FGameplayAttribute(*)()> Pair : As->TagsASMap)
		{
			LrASC->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
				[this,Pair,BarWidget,As](const FOnAttributeChangeData& Data)
				{
					const FGameplayTag* TagsAsMaxTag = As->TagsASMaxTags.Find(Pair.Key);
					float Max = 0;
					if (TagsAsMaxTag)
					{
						FAttributeFuncPtr* Find = As->TagsASMap.Find(*TagsAsMaxTag);
						if (Find)
						{
							FGameplayAttribute MaxAttribute = (*Find)();
							Max = MaxAttribute.GetNumericValue(As);
						}
					}
					BarWidget->UpdateChance(Pair.Key, Pair.Value().GetNumericValue(As), Max);
				});
		}
	}
	InitAS();
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

uint8 ALrEnemyPawn::GetClassID() const
{
	return 100;
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
