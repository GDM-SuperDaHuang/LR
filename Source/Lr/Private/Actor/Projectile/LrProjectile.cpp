// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Projectile/LrProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "ASC/LrASC.h"
#include "Actor/GC/LrDestructibleActor.h"
#include "Component/GC/LrDestructionComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
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
	Collision->SetCollisionObjectType(ECC_Projectile); //自身为Projectile类型
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 先全部忽略
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); //对于pawn，ECR_Block：阻挡的时候触发。ECR_Overlap:重叠时候触发。
	// 可破坏物：GC 可能是 Destructible / PhysicsBody / WorldStatic，全部放 Overlap 才能收到
	Collision->SetCollisionResponseToChannel(ECC_Destructible, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	Collision->SetGenerateOverlapEvents(true);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->InitialSpeed = Speed; // 初始速度
	Movement->MaxSpeed = 5000.0f; // 最大速度（想做成"加速箭"可再把 Max 提高）
	Movement->ProjectileGravityScale = 0; // 0 = 纯直线，火球冰箭常用；弓箭可把这里调 0.5~1
	Movement->bRotationFollowsVelocity = true;
	Movement->bIsHomingProjectile = true;
	// Movement->HomingAccelerationMagnitude = 3000.f; // 追踪加速度，值越大转向越迅猛

	LrNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailFX"));
	LrNiagaraComponent->SetupAttachment(RootComponent);

	LrMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LrFXMesh"));
	LrMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ALrProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (LrNiagaraComponent)
	{
		// LrNiagaraComponent->SetAsset(LrFlightFX);
		LrNiagaraComponent->Activate(true);
	}
	//设置寿命
	SetLifeSpan(5);
	SetReplicateMovement(true);
	//阻挡才触发
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ALrProjectile::OnProjectileBeginOverlap);
}

void ALrProjectile::OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

	//如果是生物
	if (ULrASC* LrASC = Cast<ULrASC>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor)))
	{
		DamageEffectParams.AddFlag(EDamageFlags::Burn);
		LrASC->ApplyDamageToTarget(ProjectileOwner, OtherActor, DamageEffectParams);
	}

	//如果是可破坏物体，则碎裂：优先走 Actor 入口（自带唤醒 + Hit 兜底）
	FVector BulletVelocity = GetVelocity();
	if (Movement && BulletVelocity.IsNearlyZero())
	{
		BulletVelocity = Movement->Velocity;
	}
	if (BulletVelocity.IsNearlyZero())
	{
		BulletVelocity = GetActorForwardVector() * FMath::Max(Speed, 600.f);
	}

	// Overlap 的 SweepResult 可能没有 ImpactPoint / Item，这里补一个可靠的 Hit
	FHitResult FixedHit(SweepResult);
	if (FixedHit.ImpactPoint.IsNearlyZero())
	{
		FixedHit.Location = GetActorLocation();
		FixedHit.ImpactPoint = GetActorLocation();
		FixedHit.ImpactNormal = (-BulletVelocity).GetSafeNormal();
		FixedHit.Normal = FixedHit.ImpactNormal;
		FixedHit.TraceStart = FixedHit.ImpactPoint - BulletVelocity.GetSafeNormal() * 10.f;
		FixedHit.TraceEnd = FixedHit.ImpactPoint;
		FixedHit.Component = OtherComp;
		FixedHit.bBlockingHit = false;
	}

	if (ALrDestructibleActor* Destructible = Cast<ALrDestructibleActor>(OtherActor))
	{
		Destructible->NotifyBulletHit(FixedHit, BulletVelocity, TestDamage);
	}
	else if (ULrDestructionComponent* Destruction = OtherActor->FindComponentByClass<ULrDestructionComponent>())
	{
		Destruction->ApplyBulletImpact(FixedHit, BulletVelocity, TestDamage);
	}

	DeactivateProjectile();
}

// void ALrProjectile::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
// {
// 	if (!HasAuthority())
// 	{
// 		return;
// 	}
//
// 	if (!OtherActor)
// 	{
// 		return;
// 	}
//
// 	if (OtherActor == ProjectileOwner)
// 	{
// 		return;
// 	}
// 	
//
// 	if (ULrASC* LrASC = Cast<ULrASC>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor)))
// 	{
// 		ALrPawnBase* OwnerPawn = Cast<ALrPawnBase>(OtherActor);
// 		if (ULrCombatComponentBase* Combat = OwnerPawn->FindComponentByClass<ULrCombatComponentBase>())
// 		{
// 			DamageEffectParams.AddFlag(EDamageFlags::Burn);
// 			LrASC->ApplyDamageToTarget(OtherActor, DamageEffectParams);
// 		}
// 	}
//
// 	DeactivateProjectile();
// }

void ALrProjectile::DeactivateProjectile()
{
	if (!HasAuthority())
	{
		return;
	}
	LrNiagaraComponent->Activate(true);
	Destroy();
}
