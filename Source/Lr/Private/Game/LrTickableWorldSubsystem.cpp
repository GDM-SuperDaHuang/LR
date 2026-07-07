// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/LrTickableWorldSubsystem.h"

#include "DefaultLevelSequenceInstanceData.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "Actor/Lightning/LrLightning.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/LrLeveSequenceDA.h"
#include "Framework/Docking/TabManager.h"
#include "Lib/LrCommonLibrary.h"
#include "Pawn/LrEnemyPawn.h"
#include "Pawn/LrHeroPawn.h"
#include "Pawn/LrPawnBase.h"
#include "Player/LrPlayerController.h"
#include "UI/Widget/Bar/WorldBar/LrWorldBarWidget.h"

TStatId ULrTickableWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULrWorldBarSubsystem, STATGROUP_Tickables);
}

/**
 * 集中刷新
 * @param DeltaTime 
 */
void ULrTickableWorldSubsystem::Tick(float DeltaTime)
{
	if (IsScare == false)
	{
		CoolTime -= DeltaTime;
	}
	if (FMath::IsNearlyEqual(CoolTime, 30.f, DeltaTime))
	{
		IsScare = true;
	}
	if (CoolTime <= 0)
	{
		CoolTime = 30.f;
	}

	for (int32 i = ActiveBars.Num() - 1; i >= 0; --i)
	{
		ULrWorldBarWidget* Bar = ActiveBars[i].Get();
		if (!Bar)
		{
			ActiveBars.RemoveAtSwap(i);
			continue;
		}
		Bar->Tick(DeltaTime);
		Bar->KeepShowTime -= DeltaTime;
		if (FMath::IsNearlyEqual(Bar->KeepShowTime, 0.f, DeltaTime)) //接近0的时候
		{
			//隐藏
			Bar->SetVisibility(ESlateVisibility::Hidden);
		}
		else if (FMath::IsNearlyEqual(Bar->KeepShowTime, 3.f, DeltaTime))
		{
			//显示
			Bar->SetVisibility(ESlateVisibility::Visible);
		}

		if (FMath::IsNearlyEqual(Bar->LastGhostPercent, Bar->TargetPercent, 0.001f))
		{
			if (FMath::IsNearlyEqual(Bar->LastGhostPercent, 0.f, 0.001f))
			{
				ActiveBars.RemoveAtSwap(i);
			}
			Bar->bIsAnimating = false;
		}
		else
		{
			Bar->bIsAnimating = true;
		}

	}
}

void ULrTickableWorldSubsystem::RegisterActiveBar(ULrWorldBarWidget* Bar)
{
	ActiveBars.AddUnique(Bar);
}

void ULrTickableWorldSubsystem::RemoveLightning(const ALrPawnBase* ALrPawnBas)
{
	LrLightningPool.Remove(ALrPawnBas);
}

ALrLightning* ULrTickableWorldSubsystem::GetLightning(ALrPawnBase* LrPawnBas)
{
	ALrLightning** LrLightning = LrLightningPool.Find(LrPawnBas);
	if (LrLightning == nullptr)
	{
		return RegisterLightning(LrPawnBas);
	}
	if (*LrLightning)
	{
		return *LrLightning;
	}
	return nullptr;
}


ALrLightning* ULrTickableWorldSubsystem::RegisterLightning(ALrPawnBase* LrPawnBase)
{
	// todo 从对象池获取
	// FActorSpawnParameters Params;
	// Params.Owner = ALrPawnBas;
	// Params.Instigator = ALrPawnBas;
	if (ALrGameModeBase* LrGameModeBase = ULrCommonLibrary::GetLrGameModeBase(LrPawnBase))
	{
		ULrGAListDA* LrGaListDa = LrGameModeBase->LrGAListDA;
		ALrLightning* LrLightning = GetWorld()->SpawnActor<ALrLightning>(LrGaListDa->LightningClass, LrPawnBase->GetActorTransform(), FActorSpawnParameters());
		LrLightning->Init(LrPawnBase);
		LrLightningPool.Add(LrPawnBase, LrLightning);
		return LrLightning;
	}
	return nullptr;
}


//////////////////////////////////////////////////////////////////////////
/// 吓人
void ULrTickableWorldSubsystem::PlayJumpScare(ALrHeroPawn* Player, ALrEnemyPawn* Monster)
{
	if (!IsScare)
	{
		return;
	}
	IsScare = false;
	ALrGameModeBase* LrGameModeBase = ULrCommonLibrary::GetLrGameModeBase(Player);
	if (LrGameModeBase)
	{
		SequenceTemplate = LrGameModeBase->LrLeveSequenceDA->SequenceTemplate;
	}

	if (!Player || !Monster || !SequenceTemplate)
		return;

	PlayerPawn = Player;
	MonsterPawn = Monster;

	SetupCamera();
	BuildRuntimeSequence();
	PlayerSeq->OnFinished.AddDynamic(
		this,
		&ULrTickableWorldSubsystem::OnJumpScareFinished
	);
	PlayerSeq->Play();
	if (UAnimInstance* AnimInstance = PlayerPawn->LrSkeletalMeshComponent->GetAnimInstance())
	{
		AnimInstance->Montage_Play(LrGameModeBase->LrLeveSequenceDA->CollapseMontage);
	}
}

void ULrTickableWorldSubsystem::BuildRuntimeSequence()
{
	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bAutoPlay = false;

	SeqActor = nullptr;
	ALevelSequenceActor* OutActor = nullptr;

	PlayerSeq = ULevelSequencePlayer::CreateLevelSequencePlayer(
		GetWorld(),
		SequenceTemplate,
		Settings,
		OutActor
	);
	SeqActor = OutActor;

	// OutActor->bOverrideInstanceData = true;
	// if (UDefaultLevelSequenceInstanceData* InstanceData = Cast<UDefaultLevelSequenceInstanceData>(SeqActor->DefaultInstanceData))
	// {
	// 	InstanceData->TransformOrigin = FTransform(FRotator::ZeroRotator, PlayerPawn->GetActorLocation(), FVector::OneVector);
	// }
	UDefaultLevelSequenceInstanceData* InstanceData = Cast<UDefaultLevelSequenceInstanceData>(SeqActor->DefaultInstanceData);
	if (!InstanceData)
	{
		InstanceData = NewObject<UDefaultLevelSequenceInstanceData>(OutActor);
		OutActor->DefaultInstanceData = InstanceData;
	}

	OutActor->bOverrideInstanceData = true;
	InstanceData->TransformOrigin = FTransform(PlayerPawn->GetActorLocation());
	//test
	UMovieScene* MovieScene = SequenceTemplate->GetMovieScene();
	if (!PlayerSeq || !OutActor)
		return;
	for (FMovieSceneBinding Binding : MovieScene->GetBindings())
	{
		const FGuid Guid = Binding.GetObjectGuid();

		if (const FMovieScenePossessable* Possessable = MovieScene->FindPossessable(Guid))
		{
			UE_LOG(LogTemp, Warning, TEXT("Possessable : %s"),
			       *Possessable->GetName());
		}
		else if (const FMovieSceneSpawnable* Spawnable = MovieScene->FindSpawnable(Guid))
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawnable : %s"),
			       *Spawnable->GetName());
		}
	}

	// 🎯 绑定 Camera
	SeqActor->SetBindingByTag("LrCinCameraActor", {RuntimeCamera});

	// 🎯 绑定 Monster
	SeqActor->SetBindingByTag("Monster", {MonsterPawn});

	// 🎯 绑定 Player
	SeqActor->SetBindingByTag("Player", {PlayerPawn});
}

void ULrTickableWorldSubsystem::SetupCamera()
{
	if (!PlayerPawn || !MonsterPawn)
		return;

	FVector FaceLoc = MonsterPawn->LrSkeletalMeshComponent->GetSocketLocation("FaceSocket");

	// 💀 贴脸核心
	FVector CamLoc = FaceLoc - MonsterPawn->GetActorForwardVector() * FaceOffset;

	FRotator CamRot = (FaceLoc - CamLoc).Rotation();

	FActorSpawnParameters Params;
	RuntimeCamera = GetWorld()->SpawnActor<ACameraActor>(CamLoc, CamRot, Params);

	ALrPlayerController* PC = PlayerPawn->GetController<ALrPlayerController>();
	if (PC)
	{
		PC->SetViewTarget(RuntimeCamera);
		PC->InhibitoryInput = true;
	}
}

void ULrTickableWorldSubsystem::OnJumpScareFinished()
{
	if (ALrPlayerController* PC = PlayerPawn->GetController<ALrPlayerController>())
	{
		PC->SetViewTarget(PlayerPawn);
		PC->InhibitoryInput = false;
	}

	if (RuntimeCamera)
	{
		RuntimeCamera->Destroy();
		RuntimeCamera = nullptr;
	}

	if (SeqActor)
	{
		SeqActor->Destroy();
		SeqActor = nullptr;
	}

	PlayerSeq = nullptr;
}

//////////////////////////////////////////////////////////////////////////
