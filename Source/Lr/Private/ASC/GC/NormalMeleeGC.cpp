// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GC/NormalMeleeGC.h"

#include "Kismet/GameplayStatics.h"
#include "Tags/LrGameplayTags.h"

UNormalMeleeGC::UNormalMeleeGC()
{
	FLrGameplayTags LrGameplayTags = FLrGameplayTags::Get();
	// ASC->ExecuteGameplayCue(TAG_Melee_Trail, Params); 触发 OnExecute_Implementation
	GameplayCueTag = LrGameplayTags.GameplayCue_Melee;
}

bool UNormalMeleeGC::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget) return false;

	AActor* SourceActor = Parameters.Instigator.Get();
	if (!SourceActor) return false;

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
