// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrHeroBoxPawn.h"

#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Nav/LrNavMovementComponent.h"

ALrHeroBoxPawn::ALrHeroBoxPawn()
{
	LrBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Capsule"));
	RootComponent = LrBoxComponent;
	// LrCapsuleComponent->InitCapsuleSize(34.f, 88.f);
	LrBoxComponent->SetCollisionProfileName(TEXT("LrHeroPawnCapsuleComponent"));

	// 相机 弹簧臂
	CameraBoom->SetupAttachment(RootComponent);

	// =========================
	// 骨骼 →碰撞体
	// =========================
	// LrSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LrMesh"));
	LrSkeletalMeshComponent->SetupAttachment(LrBoxComponent);

	// =========================
	// 武器 →骨架
	// =========================
	EquippedWeaponComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("EquippedWeapon"));
	EquippedWeaponComponent->SetupAttachment(LrSkeletalMeshComponent); // 或 RootComponent，根据需求


	// =========================
	// Mover
	// =========================
	LrMoverComponent->SetUpdatedComponent(LrBoxComponent);

	// =========================
	// Nav → Mover 桥接
	// =========================
	LrNavMoverComponent = CreateDefaultSubobject<ULrNavMovementComponent>(TEXT("NavMoverComponent"));
	LrNavMoverComponent->UpdatedComponent = LrBoxComponent;

	/**
	 * true：这个组件，是否会被 NavigationSystem 考虑为：
	 * 1，动态障碍
	 * 2，NavMesh 生成 / 更新的参与者
	 */
	LrBoxComponent->SetCanEverAffectNavigation(false);


	// =========================
	// 选中提示相关
	// =========================
	SelectionRing->SetupAttachment(RootComponent);
}
