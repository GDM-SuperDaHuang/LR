// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrEnemyBoxPawn.h"

#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Nav/LrNavMovementComponent.h"
#include "UI/Component/LrWorldWidgetComponent.h"

ALrEnemyBoxPawn::ALrEnemyBoxPawn()
{
	// =========================
	// 骨骼 →碰撞体
	// =========================
	LrBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("EnemyCapsule"));
	RootComponent = LrBoxComponent;
	LrBoxComponent->SetCollisionProfileName(TEXT("LrEnemyPawnCapsuleComponent"));
	LrBoxComponent->SetCanEverAffectNavigation(false);
	LrBoxComponent->SetCollisionObjectType(ECC_Pawn);

	LrSkeletalMeshComponent->SetupAttachment(LrBoxComponent);
	LrSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LrSkeletalMeshComponent->SetCanEverAffectNavigation(false);

	// =========================
	// 武器 →骨架
	// =========================
	// EquippedWeaponComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("EnemyEquippedWeapon"));
	// EquippedWeaponComponent->SetupAttachment(LrSkeletalMeshComponent);

	// =========================
	// Mover
	// =========================
	LrMoverComponent->SetUpdatedComponent(LrBoxComponent);

	
	// =========================
	// Nav → Mover 桥接
	// =========================
	LrNavMoverComponent->SetUpdatedComponent(LrBoxComponent);


	// =========================
	// 敌人UI
	// =========================
	LrWidgetComponent->SetupAttachment(RootComponent);


	// =========================
	// 选中提示相关
	// =========================
	SelectionRing->SetupAttachment(RootComponent);
}
