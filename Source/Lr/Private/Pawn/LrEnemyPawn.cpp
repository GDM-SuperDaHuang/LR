// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrEnemyPawn.h"

#include "AI/LrAIController.h"
#include "ASC/LrASC.h"
#include "ASC/AS/LrAS.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "MotionWarpingComponent.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Nav/LrNavMovementComponent.h"

ALrEnemyPawn::ALrEnemyPawn()
{
	PrimaryActorTick.bCanEverTick = false;

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

	EquippedWeaponComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("EnemyEquippedWeapon"));
	EquippedWeaponComponent->SetupAttachment(LrSkeletalMeshComponent);

	LrMotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("EnemyMotionWarping"));
	
	CharacterMotionComponent->SetUpdatedComponent(LrCapsuleComponent);


	LrNavMoverComponent = CreateDefaultSubobject<ULrNavMovementComponent>(TEXT("EnemyNavMoverComponent"));
	LrNavMoverComponent->UpdatedComponent = LrCapsuleComponent;

	LrEnemyASC = CreateDefaultSubobject<ULrASC>(TEXT("EnemyASC"));
	LrEnemyASC->SetIsReplicated(true);
	LrEnemyASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	LrEnemyAS = CreateDefaultSubobject<ULrAS>(TEXT("EnemyAS"));

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyHPBarWidget"));
	HealthBarWidget->SetupAttachment(LrCapsuleComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ALrAIController::StaticClass();
	bUseControllerRotationYaw = false;
}

void ALrEnemyPawn::BeginPlay()
{
	Super::BeginPlay();
}

void ALrEnemyPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	LrEnemyASC->InitAbilityActorInfo(this, this);

	LrASC = LrEnemyASC;
	LrAS = LrEnemyAS;

	OnASCRegistered.Broadcast(LrASC);

	if (StartupAbilities.Num() > 0)
	{
		LrEnemyASC->AddGA(StartupAbilities);
	}
}