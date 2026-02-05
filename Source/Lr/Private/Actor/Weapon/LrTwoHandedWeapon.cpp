// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Weapon/LrTwoHandedWeapon.h"

#include "NiagaraComponent.h"
#include "Data/LrGAListDA.h"
#include "Lib/LrCommonLibrary.h"
#include "Tags/LrGameplayTags.h"


ALrTwoHandedWeapon::ALrTwoHandedWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponLSKM = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponLSKM"));
	// WeaponLSKM->SetupAttachment(RootComponent, HandSocketLName);
	WeaponLSKM->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponRSKM = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponRSKM"));
	// WeaponRSKM->SetupAttachment(RootComponent, HandSocketRName);
	WeaponRSKM->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponComponent->SetupAttachment(WeaponLSKM);
	WeaponComponent->SetAutoActivate(false); // 默认不播放

	WeaponRComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WeaponRComponent"));
	WeaponRComponent->SetupAttachment(WeaponRSKM);
	WeaponRComponent->SetAutoActivate(false); // 默认不播放
}

void ALrTwoHandedWeapon::OnEquipped(AActor* OwnerPawn, FLrWeaponConfig WeaponConfig)
{
	if (!WeaponComponent)
	{
		return;
	}

	if (!TrailNS)
	{
		FLrGameplayTags LrGameplayTags = FLrGameplayTags::Get();
		FLrNSConfig LrDAConfig = ULrCommonLibrary::FindNSByTag(OwnerPawn, LrGameplayTags.GameplayCue_Melee_Trail);
		if (LrDAConfig.NS != nullptr)
		{
			TrailNS = LrDAConfig.NS;
		}
	}

	if (TrailNS)
	{
		WeaponComponent->SetAsset(TrailNS);
		WeaponComponent->Deactivate(); // 停止特效
		WeaponRComponent->SetAsset(TrailNS);
		WeaponRComponent->Deactivate();
	}

	// todo
	WeaponLSKM->SetSkeletalMeshAsset(WeaponConfig.WeaponSMList[0]);
	WeaponRSKM->SetSkeletalMeshAsset(WeaponConfig.WeaponSMList[1]);

	if (USkeletalMeshComponent* OwnerMesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>())
	{
		WeaponLSKM->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocketLName);
		WeaponRSKM->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocketRName);
	}
}

void ALrTwoHandedWeapon::OnUnequipped()
{
	if (WeaponComponent)
	{
		WeaponComponent->Deactivate();
	}
	if (WeaponRComponent)
	{
		WeaponRComponent->Deactivate();
	}

	// 2️⃣ 左手武器脱离
	if (WeaponLSKM)
	{
		WeaponLSKM->DetachFromComponent(
			FDetachmentTransformRules::KeepWorldTransform
		);
	}

	// 3️⃣ 右手武器脱离
	if (WeaponRSKM)
	{
		WeaponRSKM->DetachFromComponent(
			FDetachmentTransformRules::KeepWorldTransform
		);
	}


	// DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}
