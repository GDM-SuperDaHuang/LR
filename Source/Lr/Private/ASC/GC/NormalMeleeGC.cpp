// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GC/NormalMeleeGC.h"

#include "NiagaraFunctionLibrary.h"
#include "Data/LrGAListDA.h"
#include "Kismet/GameplayStatics.h"
#include "Lib/LrCommonLibrary.h"
#include "Pawn/LrPawnBase.h"
#include "Tags/LrGameplayTags.h"

UNormalMeleeGC::UNormalMeleeGC()
{
	FLrGameplayTags LrGameplayTags = FLrGameplayTags::Get();
	// ASC->ExecuteGameplayCue(TAG_Melee_Trail, Params); 触发 OnExecute_Implementation
	// GameplayCueTag = LrGameplayTags.GameplayCue_Melee;
}

bool UNormalMeleeGC::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return false;

	AActor* SourceActor = Parameters.Instigator.Get();
	if (!SourceActor) return false;


	ALrPawnBase* Pawn = Cast<ALrPawnBase>(MyTarget);
	if (!Pawn) return false;

	TObjectPtr<USkeletalMeshComponent> Mesh = Pawn->LrSkeletalMeshComponent;
	if (!Mesh) return false;
	
	const FLrNSConfig& LrNSConfig = ULrCommonLibrary::FindNSByTag(SourceActor,Parameters.OriginalTag);
	TObjectPtr<UNiagaraSystem> WeaponTrailNiagara = LrNSConfig.NS;
	if (WeaponTrailNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
		WeaponTrailNiagara,
		Mesh,
		FName("WeaponSocket"),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		true
	);
	}
	
	// 1. 播放受击特效
	// UGameplayStatics::SpawnEmitterAtLocation(
	// 	MyTarget->GetWorld(),
	// 	HitEffect,
	// 	Parameters.Location
	// );
	//
	// // 2. 播放受击音效
	// UGameplayStatics::PlaySoundAtLocation(
	// 	MyTarget,
	// 	HitSound,
	// 	Parameters.Location
	// );
	
	return true;
}
