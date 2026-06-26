// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrEnemyPawn.h"

#include "ASC/LrASC.h"
#include "ASC/AS/LrAS.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "Actor/Corpse/LrCorpseActor.h"
#include "AI/Controller/LrAIControllerBase.h"
#include "AI/ST/LrSTComponent.h"
#include "AI/Subsystem/LrAIManagerSubsystem.h"
#include "Alembic/AbcGeom/IFaceSet.h"
#include "Animation/AnimInstance.h"
#include "Component/LrPatrolRouteComponent.h"
#include "Component/Combat/LrAICombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
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

	// LrSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LrEnemyMesh"));
	LrSkeletalMeshComponent->SetupAttachment(LrCapsuleComponent);
	// LrSkeletalMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	// LrSkeletalMeshComponent->SetOnlyOwnerSee(false);
	// LrSkeletalMeshComponent->SetOwnerNoSee(false);
	LrSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LrSkeletalMeshComponent->SetCanEverAffectNavigation(false);
	
	// =========================
	// 武器 →骨架
	// =========================
	EquippedWeaponComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("EnemyEquippedWeapon"));
	EquippedWeaponComponent->SetupAttachment(LrSkeletalMeshComponent);
	
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

	// =========================
	// 选中提示相关
	// =========================
	SelectionRing->SetupAttachment(RootComponent);
	

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
	BarWidget->InitWidget();
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
	// FPawnTypeGAConfig LrDAConfig = ULrCommonLibrary::FindPawnTypeGAConfig(this, PawnType);
	// LrASC->AddGA(LrDAConfig.GATagList);
	LrASC->AddAllGA(this);
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



void ALrEnemyPawn::ToDie(const FLrDieParameters& DieParam)
{
	bIsDead = true;
	// 隐藏组件（不再渲染）
	LrWidgetComponent->SetVisibility(false);
	// 同时隐藏内部 UUserWidget，防止它被其他系统激活
	if (UUserWidget* BarWidget = LrWidgetComponent->GetUserWidgetObject())
	{
		BarWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	if (DieParam.DeathMontage)
	{
		PlayDeathMontage(DieParam.DeathMontage);
	}
	else
	{
		FinishDeath();
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		// 强制取消所有技能，这样攻击技能就会提前走 EndAbility，而不会在死后还尝试去绑定 Task
		ASC->CancelAllAbilities();
	}
	if (ALrAIControllerBase* AI = Cast<ALrAIControllerBase>(GetController()))
	{
		// 禁止移动
		AI->StopMovement();
		// 尝试寻找 StateTree 组件（通常挂在 AIController 上）
		if (UStateTreeComponent* StateTreeComp = AI->FindComponentByClass<UStateTreeComponent>())
		{
			// 显式停止 StateTree 运行，防止其继续 Tick 并访问已死亡的 Pawn
			StateTreeComp->StopLogic(TEXT("Pawn Died"));
		}
		// 安全地解除控制
		AI->UnPossess();
	}

	
	FLrCorpseConfig CorpseConfig = ULrCommonLibrary::FindCorpseConfigByPawnType(this, DieParam.PawnType);
	
	// ========== 3. 播放近战 Montage ==========
	UAnimInstance* AnimInstance = LrSkeletalMeshComponent->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CorpseConfig.CorpseMontage);
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ALrEnemyPawn::OnAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, CorpseConfig.CorpseMontage);
	}	

	// ====== 2. 核心：生成独立的尸体 Actor ======
	USkeletalMeshComponent* NewSMComponent = nullptr;
	if (ALrCorpseActor* CorpseActor = GetWorld()->SpawnActor<ALrCorpseActor>(ALrCorpseActor::StaticClass(), GetActorLocation(), GetActorRotation()))
	{
		// 让尸体复制当前 Mesh 的动作，并飞出去
		NewSMComponent = CorpseActor->InitializeCorpse(LrSkeletalMeshComponent, DieParam.DeathImpulse, CorpseConfig);
	}

	// ====== 3. 本体干净利落地进入“隐藏”状态，准备返回对象池 ======
	// 关动画蓝图
	LrSkeletalMeshComponent->SetAnimInstanceClass(nullptr);
	// 关掉 Mover 和自身碰撞（Mover 再也不会报任何警告，因为原 Mesh 根本没开物理）
	if (LrMoverComponent)
	{
		LrMoverComponent->SetComponentTickEnabled(false);
		LrMoverComponent->Deactivate();
	}
	LrCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LrSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 隐身
	SetActorHiddenInGame(true);

	// ====== 4. 通知你的对象池管理器 ======
	// MyObjectPoolManager->ReturnToPool(this);


	// 2. 创建动态材质实例 (假设Mesh上使用的是支持溶解的材质)
	// 2. 多材质溶解逻辑
	if (NewSMComponent)
	{
		// 清空旧数组（防止复活等逻辑重复调用导致常驻内存）
		DissolveMIDs.Empty();
		// 获取当前 Mesh 的材质槽位总数
		const int32 NumMaterials = NewSMComponent->GetNumMaterials();
		for (int32 i = 0; i < NumMaterials; ++i)
		{
			// 为每一个槽位创建动态材质实例（它会自动应用到 Mesh 上）
			UMaterialInstanceDynamic* MID = NewSMComponent->CreateDynamicMaterialInstance(i);
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

void ALrEnemyPawn::OnAttackMontageEnded(UAnimMontage* AnimMontage, bool bInterrupted)
{
	if (bInterrupted)
	{
		UE_LOG(LogTemp, Warning, TEXT("Montage 被打断"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Montage 正常结束"));
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


void ALrEnemyPawn::PlayDeathMontage(UAnimMontage* Montage)
{
	UAnimInstance* Anim = LrSkeletalMeshComponent->GetAnimInstance();

	if (!Anim)
	{
		FinishDeath();
		return;
	}

	float Duration = Anim->Montage_Play(Montage);

	if (Duration <= 0.f)
	{
		FinishDeath();
		return;
	}

	FTimerHandle Handle;

	GetWorldTimerManager().SetTimer(
		Handle,
		this,
		&ALrEnemyPawn::FinishDeath,
		Duration,
		false);
}

void ALrEnemyPawn::FinishDeath()
{
	// SpawnCorpse();
	SetActorHiddenInGame(true);
}
