// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Projectile/LrProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "ASC/LrASC.h"
#include "Component/Combat/LrCombatComponentBase.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Data/LrExcelConfig.h"
#include "Engine/World.h"
#include "Game/LrTickableWorldSubsystem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Lr/Lr.h"
#include "Mover/LrMoverComponent.h"
#include "Net/UnrealNetwork.h"
#include "Pawn/LrPawnBase.h"

// Sets default values
ALrProjectile::ALrProjectile()
{
	// 1. 关闭 Tick， projectile 一般靠 MovementComponent 推进，不需要每帧回调
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; //需要优化。添加复制Actor运动状态，FRepMovement

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision); // 根组件必须是碰撞体，否则 MovementComponent 会警告
	Collision->SetCollisionProfileName("Projectile");
	Collision->SetCollisionObjectType(ECC_Projectile);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 先全部忽略
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 打玩家/敌人全靠这一行


	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->InitialSpeed = 550.0f; // 初始速度
	Movement->MaxSpeed = 550.0f; // 最大速度（想做成“加速箭”可再把 Max 提高）
	Movement->ProjectileGravityScale = 0; // 0 = 纯直线，火球冰箭常用；弓箭可把这里调 0.5~1
	Movement->bRotationFollowsVelocity = true;
	Movement->bIsHomingProjectile = true;
}

// Called when the game starts or when spawned
void ALrProjectile::BeginPlay()
{
	Super::BeginPlay();
	//设置寿命
	SetLifeSpan(5);
	SetReplicateMovement(true);
	// Collision->OnComponentHit.AddDynamic(this, &ALrProjectile::OnProjectileHit);
	Collision->OnComponentHit.AddDynamic(this, &ALrProjectile::OnProjectileHit);
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
	}
}

void ALrProjectile::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor)
	{
		return;
	}

	if (OtherActor == ProjectileOwner)
	{
		return;
	}

	// UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	// UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}

	if (ULrASC* LrASC = Cast<ULrASC>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor)))
	{
		ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(OtherActor);
		if (ULrCombatComponentBase* Combat = OwnerPawn->FindComponentByClass<ULrCombatComponentBase>())
		{
			LrASC->ApplyDamageToTarget(OtherActor, DamageEffectParams);
			OwnerPawn->LrMoverComponent->bIsInAttackWarp = true;
		}
	}
	DeactivateProjectile();
}

void ALrProjectile::DeactivateProjectile()
{
	if (!HasAuthority())
	{
		return;
	}

	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}

	GetWorldTimerManager().ClearTimer(LifeTimer);
	Movement->StopMovementImmediately();
	Movement->SetUpdatedComponent(nullptr);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	GetWorldTimerManager().ClearAllTimersForObject(this);

	AActor::SetReplicateMovement(false); //需要优化。添加复制Actor运动状态，FRepMovement
	
}
