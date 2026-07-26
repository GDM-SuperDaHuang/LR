// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrHeroCapsulePawn.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Nav/LrNavMovementComponent.h"

ALrHeroCapsulePawn::ALrHeroCapsulePawn()
{
	LrCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	RootComponent = LrCapsuleComponent;
	// LrCapsuleComponent->InitCapsuleSize(34.f, 88.f);
	LrCapsuleComponent->SetCollisionProfileName(TEXT("LrHeroPawnCapsuleComponent"));

	// =========================
	// 骨骼 →碰撞体
	// =========================
	// LrSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LrMesh"));
	LrSkeletalMeshComponent->SetupAttachment(LrCapsuleComponent);

	// =========================
	// 武器 →骨架
	// =========================
	EquippedWeaponComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("EquippedWeapon"));
	EquippedWeaponComponent->SetupAttachment(LrSkeletalMeshComponent); // 或 RootComponent，根据需求


	// =========================
	// Mover
	// =========================
	LrMoverComponent->SetUpdatedComponent(LrCapsuleComponent);

	// =========================
	// Nav → Mover 桥接
	// =========================
	LrNavMoverComponent = CreateDefaultSubobject<ULrNavMovementComponent>(TEXT("NavMoverComponent"));
	LrNavMoverComponent->UpdatedComponent = LrCapsuleComponent;

	/**
	 * true：这个组件，是否会被 NavigationSystem 考虑为：
	 * 1，动态障碍
	 * 2，NavMesh 生成 / 更新的参与者
	 */
	LrCapsuleComponent->SetCanEverAffectNavigation(false);


	// =========================
	// 选中提示相关
	// =========================
	SelectionRing->SetupAttachment(RootComponent);
}
