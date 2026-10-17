// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/GC/LrDestructibleActor.h"

#include "Component/GC/LrDestructionComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

// Sets default values
ALrDestructibleActor::ALrDestructibleActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	RootComponent = GeometryCollection;
	Destruction = CreateDefaultSubobject<ULrDestructionComponent>(TEXT("Destruction"));
	Destruction->SetGeometryCollection(GeometryCollection);
	//防止一开始就碎裂：Dynamic + StartAwake=false -> 物理体初始睡眠（Static 粒子不参与碎裂判定，命中后什么都不会发生）
	GeometryCollection->SetSimulatePhysics(true);
	GeometryCollection->ObjectType = EObjectStateTypeEnum::Chaos_Object_Dynamic;
	GeometryCollection->BodyInstance.bStartAwake = false;
	GeometryCollection->SetEnableGravity(true);
	GeometryCollection->SetCollisionProfileName(TEXT("BlockAll"));
	// 子弹是 QueryOnly Overlap，GC 侧必须对 Projectile / Destructible 放 Overlap 才能收到
	GeometryCollection->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
	GeometryCollection->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
	GeometryCollection->SetCollisionResponseToChannel(ECC_Destructible, ECR_Overlap);
	GeometryCollection->SetGenerateOverlapEvents(true);
	GeometryCollection->SetNotifyRigidBodyCollision(true);
}

void ALrDestructibleActor::BeginPlay()
{
	Super::BeginPlay();

	// Actor BeginPlay 早于组件 BeginPlay，测试破坏前先把破坏阈值切到游戏数值。
	if (Destruction)
	{
		Destruction->ApplyDamageSettings();
	}

	if (!bTestBreak)
	{
		TestBreak();
	}
	
}

void ALrDestructibleActor::TestBreak() const
{
	if (!Destruction || !GeometryCollection)
	{
		return;
	}

	const FVector HitPoint = GetActorLocation();

	const FVector Direction = FVector::ForwardVector;

	Destruction->ApplyImpactSimple(
		HitPoint,
		Direction,
		1500.f,
		100.f);
}

bool ALrDestructibleActor::NotifyBulletHit(const FHitResult& Hit, const FVector& BulletVelocity, float Damage)
{
	if (!Destruction || !GeometryCollection)
	{
		return false;
	}

	// Overlap 的 Hit 可能没有 ImpactPoint，用子弹位置 / 自身位置兜底
	FVector HitPoint = Hit.ImpactPoint;
	if (HitPoint.IsNearlyZero())
	{
		HitPoint = Hit.Location;
	}

	FVector Direction = BulletVelocity.GetSafeNormal();
	if (Direction.IsNearlyZero() && !Hit.ImpactNormal.IsNearlyZero())
	{
		Direction = (-Hit.ImpactNormal).GetSafeNormal();
	}

	FHitResult FixedHit(Hit);
	if (FixedHit.ImpactPoint.IsNearlyZero())
	{
		FixedHit.ImpactPoint = HitPoint.IsNearlyZero() ? GetActorLocation() : HitPoint;
		FixedHit.Location = FixedHit.ImpactPoint;
		FixedHit.Component = GeometryCollection;
	}

	FVector FixedVelocity = Direction.IsNearlyZero() ? FVector::ForwardVector * 1000.f : Direction * FMath::Max(BulletVelocity.Size(), 1000.f);

	// WakeUp + Strain + BreakingVelocity 都在 Destruction 里做
	return Destruction->ApplyBulletImpact(FixedHit, FixedVelocity, Damage);
}
