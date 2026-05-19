// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Subsystem/LrAIManagerSubsystem.h"

#include "AI/AISettings/LrAISettings.h"
#include "AI/Controller/LrBossAIController.h"
#include "AI/Controller/LrNormalAIController.h"
#include "Pawn/LrEnemyPawn.h"

void ULrAIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NormalAIClass = ALrNormalAIController::StaticClass();
	BossAIClass = ALrBossAIController::StaticClass();
}

ALrAIControllerBase* ULrAIManagerSubsystem::SpawnNormalAI(ALrEnemyPawn* Enemy)
{
	const ULrAISettings* Settings = GetDefault<ULrAISettings>();
	if (!Settings)
	{
		return nullptr;
	}

	TSubclassOf<ALrAIControllerBase> ControllerClass = Settings->NormalAIControllerClass.LoadSynchronous();
	return SpawnController(Enemy,ControllerClass);
}

ALrAIControllerBase* ULrAIManagerSubsystem::SpawnBossAI(ALrEnemyPawn* Enemy)
{
	const ULrAISettings* Settings = GetDefault<ULrAISettings>();
	if (!Settings)
	{
		return nullptr;
	}
	TSubclassOf<ALrAIControllerBase> ControllerClass = Settings->NormalAIControllerClass.LoadSynchronous();
	return SpawnController(Enemy,ControllerClass);
}

void ULrAIManagerSubsystem::SwitchToBossAI(ALrEnemyPawn* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	ALrAIControllerBase* OldController =Cast<ALrAIControllerBase>(Enemy->GetController());

	if (OldController)
	{
		OldController->ShutdownAI();

		OldController->UnPossess();

		OldController->Destroy();
	}

	SpawnBossAI(Enemy);
}

void ULrAIManagerSubsystem::SwitchToNormalAI(ALrEnemyPawn* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	ALrAIControllerBase* OldController =Cast<ALrAIControllerBase>(Enemy->GetController());

	if (OldController)
	{
		OldController->ShutdownAI();

		OldController->UnPossess();

		OldController->Destroy();
	}

	SpawnNormalAI(Enemy);
}

ALrAIControllerBase* ULrAIManagerSubsystem::SpawnController(ALrEnemyPawn* Enemy, TSubclassOf<ALrAIControllerBase> ControllerClass)
{
	if (!Enemy)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ALrAIControllerBase* NewController =
		World->SpawnActor<ALrAIControllerBase>(
			ControllerClass,
			Enemy->GetActorLocation(),
			Enemy->GetActorRotation());

	if (!NewController)
	{
		return nullptr;
	}

	NewController->Possess(Enemy);

	return NewController;
}
