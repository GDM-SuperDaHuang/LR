// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LrAIManagerSubsystem.generated.h"

class ALrAIControllerBase;
class ALrEnemyPawn;
/**
 * 
 */
UCLASS()
class LR_API ULrAIManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	ALrAIControllerBase* SpawnNormalAI(ALrEnemyPawn* Enemy);

	ALrAIControllerBase* SpawnBossAI(ALrEnemyPawn* Enemy);

	void SwitchToBossAI(ALrEnemyPawn* Enemy);

	void SwitchToNormalAI(ALrEnemyPawn* Enemy);

protected:

	ALrAIControllerBase* SpawnController(ALrEnemyPawn* Enemy,TSubclassOf<ALrAIControllerBase> ControllerClass);

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ALrAIControllerBase> NormalAIClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ALrAIControllerBase> BossAIClass;
};
