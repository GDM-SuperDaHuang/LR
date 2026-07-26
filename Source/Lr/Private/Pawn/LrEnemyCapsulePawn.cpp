// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrEnemyCapsulePawn.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Mover/Nav/LrNavMovementComponent.h"
#include "UI/Component/LrWorldWidgetComponent.h"

ALrEnemyCapsulePawn::ALrEnemyCapsulePawn()
{
	// =========================
	// 骨骼 →碰撞体
	// =========================
	LrCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("EnemyCapsule"));
	RootComponent = LrCapsuleComponent;
	LrCapsuleComponent->InitCapsuleSize(34.f, 88.f);
	LrCapsuleComponent->SetCollisionProfileName(TEXT("LrEnemyPawnCapsuleComponent"));
	LrCapsuleComponent->SetCanEverAffectNavigation(false);
	LrCapsuleComponent->SetCollisionObjectType(ECC_Pawn);

	LrSkeletalMeshComponent->SetupAttachment(LrCapsuleComponent);
	LrSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LrSkeletalMeshComponent->SetCanEverAffectNavigation(false);

	// =========================
	// 武器 →骨架
	// =========================
	// EquippedWeaponComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("EnemyEquippedWeapon"));
	// EquippedWeaponComponent->SetupAttachment(LrSkeletalMeshComponent);

	// =========================
	// Nav → Mover 桥接
	// =========================
	LrNavMoverComponent->SetUpdatedComponent(LrCapsuleComponent);

	// =========================
	// 敌人UI
	// =========================
	LrWidgetComponent->SetupAttachment(RootComponent);
	// =========================
	// 选中提示相关
	// =========================
	SelectionRing->SetupAttachment(RootComponent);
}
