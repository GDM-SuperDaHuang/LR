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
#include "Component/LrPatrolRouteComponent.h"
#include "Component/Combat/LrAICombatComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Lib/LrCommonLibrary.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Air/LrAirMovementMode.h"
#include "Mover/Nav/LrNavMovementComponent.h"
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

void ALrEnemyPawn::ToDie(const FVector& DeathImpulse, float Duration)
{
	Super::ToDie(DeathImpulse, Duration);
	// // 1. 禁用碰撞和移动，防止死尸挡路或继续移动
	LrCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LrSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (ALrAIControllerBase* AI = Cast<ALrAIControllerBase>(GetController()))
	{
		// 禁止移动
		AI->StopMovement();
	}

	// 2. 创建动态材质实例 (假设Mesh上使用的是支持溶解的材质)
	// 2. 多材质溶解逻辑
	if (LrSkeletalMeshComponent)
	{
		// 清空旧数组（防止复活等逻辑重复调用导致常驻内存）
		DissolveMIDs.Empty();
		// 获取当前 Mesh 的材质槽位总数
		const int32 NumMaterials = LrSkeletalMeshComponent->GetNumMaterials();
		for (int32 i = 0; i < NumMaterials; ++i)
		{
			// 为每一个槽位创建动态材质实例（它会自动应用到 Mesh 上）
			UMaterialInstanceDynamic* MID = LrSkeletalMeshComponent->CreateDynamicMaterialInstance(i);
			if (MID)
			{
				DissolveMIDs.Add(MID);
			}
		}

		// 如果成功创建了至少一个动态材质
		if (DissolveMIDs.Num() > 0)
		{
			DissolveStartTime = GetWorld()->GetTimeSeconds();
			// 启动定时器
			GetWorld()->GetTimerManager().SetTimer(
				DissolveTimerHandle,
				this,
				&ALrEnemyPawn::UpdateDissolveProgress,
				0.02f,
				true
			);
		}
		else
		{
			Destroy();
		}
	}
	else
	{
		Destroy();
	}
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
