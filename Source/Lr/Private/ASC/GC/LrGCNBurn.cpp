// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GC/LrGCNBurn.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
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
	if (!MyTarget)
	{
		return false;
	}

	ALrPawnBase* LrPawn = Cast<ALrPawnBase>(MyTarget);
	if (!LrPawn)
	{
		return false;
	}

	USkeletalMeshComponent* Mesh = LrPawn->LrSkeletalMeshComponent;
	if (!Mesh)
	{
		return false;
	}

	// 创建 Niagara
	if (BurnNiagara)
	{
		NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			BurnNiagara,
			Mesh,
			TEXT("spine_03"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false);
		UpdateBurnIntensity(Parameters);
	}

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
	// 关闭 Niagara
	if (NiagaraComponent)
	{
		NiagaraComponent->Deactivate();
		NiagaraComponent = nullptr;
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
	if (!NiagaraComponent)
	{
		return;
	}

	// StackCount 通常存在 RawMagnitude
	float Intensity = FMath::Max(
		Parameters.RawMagnitude, //GameplayEffect 的 Magnitude
		1.f);

	// 给 Niagara 设置一个运行时参数
	NiagaraComponent->SetVariableFloat(TEXT("BurnIntensity"), Intensity); 
}
