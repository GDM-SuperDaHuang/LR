// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Weapon/LrWeaponBase.h"

#include "NiagaraComponent.h"
#include "Data/LrGAListDA.h"

// Sets default values
ALrWeaponBase::ALrWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	// WeaponSKM = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSKM"));
	// // 挂到右手插槽，可在子类改 Socket 名，注意名称 WeaponHandSocket 一定要一致
	// WeaponSKM->SetupAttachment(RootComponent);
	// // 武器本身不产生物理碰撞
	// WeaponSKM->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WeaponComponent"));
	// WeaponComponent->SetupAttachment(WeaponSKM);
	// WeaponComponent->SetAutoActivate(false); // 默认不播放
	SetReplicates(true); // 多人需要
}

void ALrWeaponBase::OnEquipped(AActor* OwnerPawn, FLrWeaponConfig WeaponConfig)
{
	// if (!WeaponComponent)
	// {
	// 	return;
	// }
	// if (TrailNS)
	// {
	// 	WeaponComponent->SetAsset(TrailNS);
	// 	WeaponComponent->Deactivate(); // 停止特效
	// }
}

void ALrWeaponBase::OnUnequipped()
{
	if (WeaponComponent)
	{
		WeaponComponent->Deactivate();
	}
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}
