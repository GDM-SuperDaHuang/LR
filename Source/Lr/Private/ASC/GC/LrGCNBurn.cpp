// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GC/LrGCNBurn.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Pawn/LrPawnBase.h"

ALrGCNBurn::ALrGCNBurn()
{
	// 持续型Cue
	bAutoDestroyOnRemove = true; //自动销毁
	// 允许多个实例
	bUniqueInstancePerInstigator = false;
	bUniqueInstancePerSourceObject = false;
}

bool ALrGCNBurn::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	FGameplayTag CueTag = Parameters.MatchedTagName;

	if (!MyTarget)
	{
		return false;
	}
	// 目标对象
	LrPawn = Cast<ALrPawnBase>(MyTarget);
	if (!LrPawn)
	{
		return false;
	}

	USkeletalMeshComponent* Mesh = LrPawn->LrSkeletalMeshComponent;
	if (!Mesh)
	{
		return false;
	}
	UNiagaraSystem* NiagaraSystem = LrPawn->BurnFX->GetAsset();
	if (!NiagaraSystem)
	{
		return false;
	}
	// 创建 Niagara
	LrPawn->BurnFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraSystem,
		Mesh,
		TEXT("spine_03"),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		true);
	
	UpdateBurnIntensity(Parameters);

	// 播放循环音效
	if (BurnLoopSound)
	{
		AudioComponent = UGameplayStatics::SpawnSoundAttached(
			BurnLoopSound,
			Mesh,
			TEXT("spine_03")); //骨骼位置
	}

	return true;
}

bool ALrGCNBurn::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	FGameplayTag CueTag = Parameters.MatchedTagName;

	if (!LrPawn)
	{
		return false;
	}

	if (LrPawn->BurnFX)
	{
		LrPawn->BurnFX->SetVisibility(false);
		// LrPawn->BurnFX->Deactivate();
		// LrPawn->BurnFX = nullptr;
	}

	// 停止音效
	if (AudioComponent)
	{
		AudioComponent->Stop();
		AudioComponent = nullptr;
	}

	return true;
}

bool ALrGCNBurn::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	UpdateBurnIntensity(Parameters);

	return true;
}

void ALrGCNBurn::UpdateBurnIntensity(const FGameplayCueParameters& Parameters)
{
	if (!LrPawn->BurnFX)
	{
		return;
	}

	// StackCount 通常存在 RawMagnitude
	float Intensity = FMath::Max(
		Parameters.RawMagnitude, //GameplayEffect 的 Magnitude
		1.f);

	// 给 Niagara 设置一个运行时参数
	LrPawn->BurnFX->SetVariableFloat(TEXT("BurnIntensity"), Intensity);
}
