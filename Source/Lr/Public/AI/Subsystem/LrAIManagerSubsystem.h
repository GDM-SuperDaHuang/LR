// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LrAIManagerSubsystem.generated.h"

class ALrAIControllerBase;
class ALrEnemyPawn;

/**
 * AI 控制器对象池配置
 */
USTRUCT()
struct FLrAIControllerPool
{
	GENERATED_BODY()

	// 可用（未使用）的控制器队列
	UPROPERTY()
	TArray<TObjectPtr<ALrAIControllerBase>> AvailableControllers;

	// 当前活跃的控制器（用于快速查找和统计）
	UPROPERTY()
	TArray<TWeakObjectPtr<ALrAIControllerBase>> ActiveControllers;

	// 池的最大容量（防止无限增长）
	int32 MaxPoolSize = 32;

	// 预创建数量
	int32 PreCreateCount = 4;

	void ReturnToPool(ALrAIControllerBase* Controller);
	ALrAIControllerBase* GetFromPool(UWorld* World, TSubclassOf<ALrAIControllerBase> ControllerClass);
	void PreCreateControllers(UWorld* World, TSubclassOf<ALrAIControllerBase> ControllerClass);
	void CleanupInvalidEntries();
};

/**
 * AI 管理子系统 - 带对象池
 * 负责 AI 控制器的生成、回收和切换，使用对象池避免频繁的 SpawnActor/Destroy
 */
UCLASS()
class LR_API ULrAIManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	ALrAIControllerBase* SpawnNormalAI(ALrEnemyPawn* Enemy);

	ALrAIControllerBase* SpawnBossAI(ALrEnemyPawn* Enemy);

	void SwitchToBossAI(ALrEnemyPawn* Enemy);

	void SwitchToNormalAI(ALrEnemyPawn* Enemy);

	/** 将控制器回收到对象池（替代 Destroy） */
	void RecycleController(ALrAIControllerBase* Controller);

	/** 预创建一批控制器到池中（可在游戏开始时调用） */
	void PreWarmPools();

protected:

	ALrAIControllerBase* AcquireController(ALrEnemyPawn* Enemy, TSubclassOf<ALrAIControllerBase> ControllerClass, FLrAIControllerPool& Pool);

	void ReleaseController(ALrAIControllerBase* Controller, FLrAIControllerPool& Pool);

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ALrAIControllerBase> NormalAIClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ALrAIControllerBase> BossAIClass;

	// 普通 AI 控制器对象池
	UPROPERTY()
	FLrAIControllerPool NormalAIPool;

	// Boss AI 控制器对象池
	UPROPERTY()
	FLrAIControllerPool BossAIPool;
};
