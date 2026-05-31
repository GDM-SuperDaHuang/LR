// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Subsystem/LrAIManagerSubsystem.h"

#include "AI/AISettings/LrAISettings.h"
#include "AI/Controller/LrBossAIController.h"
#include "AI/Controller/LrNormalAIController.h"
#include "Pawn/LrEnemyPawn.h"

// ==================== FLrAIControllerPool 实现 ====================

void FLrAIControllerPool::ReturnToPool(ALrAIControllerBase* Controller)
{
	if (!Controller || AvailableControllers.Num() >= MaxPoolSize)
	{
		if (Controller)
		{
			Controller->Destroy();
		}
		return;
	}

	// 从活跃列表移除
	ActiveControllers.Remove(Controller);

	// 重置控制器状态
	Controller->UnPossess();
	Controller->SetActorHiddenInGame(true);
	Controller->SetActorTickEnabled(false);

	AvailableControllers.Add(Controller);
}

ALrAIControllerBase* FLrAIControllerPool::GetFromPool(UWorld* World, TSubclassOf<ALrAIControllerBase> ControllerClass)
{
	CleanupInvalidEntries();

	// 优先从池中获取
	while (AvailableControllers.Num() > 0)
	{
		ALrAIControllerBase* Controller = AvailableControllers.Pop();
		if (IsValid(Controller))
		{
			Controller->SetActorHiddenInGame(false);
			Controller->SetActorTickEnabled(true);
			ActiveControllers.Add(Controller);
			return Controller;
		}
	}

	// 池为空，创建新的
	if (!World || !ControllerClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ALrAIControllerBase* NewController = World->SpawnActor<ALrAIControllerBase>(
		ControllerClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams);

	if (NewController)
	{
		ActiveControllers.Add(NewController);
	}

	return NewController;
}

void FLrAIControllerPool::PreCreateControllers(UWorld* World, TSubclassOf<ALrAIControllerBase> ControllerClass)
{
	if (!World || !ControllerClass)
	{
		return;
	}

	for (int32 i = 0; i < PreCreateCount; ++i)
	{
		if (AvailableControllers.Num() >= MaxPoolSize)
		{
			break;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ALrAIControllerBase* NewController = World->SpawnActor<ALrAIControllerBase>(
			ControllerClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams);

		if (NewController)
		{
			NewController->SetActorHiddenInGame(true);
			NewController->SetActorTickEnabled(false);
			AvailableControllers.Add(NewController);
		}
	}
}

void FLrAIControllerPool::CleanupInvalidEntries()
{
	// 清理活跃列表中已失效的引用
	for (int32 i = ActiveControllers.Num() - 1; i >= 0; --i)
	{
		if (!ActiveControllers[i].IsValid())
		{
			ActiveControllers.RemoveAtSwap(i);
		}
	}
}

// ==================== ULrAIManagerSubsystem 实现 ====================

void ULrAIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NormalAIClass = ALrNormalAIController::StaticClass();
	BossAIClass = ALrBossAIController::StaticClass();
}

void ULrAIManagerSubsystem::Deinitialize()
{
	// 清理对象池，销毁所有控制器
	for (ALrAIControllerBase* Controller : NormalAIPool.AvailableControllers)
	{
		if (IsValid(Controller))
		{
			Controller->Destroy();
		}
	}
	NormalAIPool.AvailableControllers.Empty();

	for (ALrAIControllerBase* Controller : BossAIPool.AvailableControllers)
	{
		if (IsValid(Controller))
		{
			Controller->Destroy();
		}
	}
	BossAIPool.AvailableControllers.Empty();

	Super::Deinitialize();
}

void ULrAIManagerSubsystem::PreWarmPools()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const ULrAISettings* Settings = GetDefault<ULrAISettings>();
	if (!Settings)
	{
		return;
	}

	TSubclassOf<ALrAIControllerBase> NormalClass = Settings->NormalAIControllerClass.LoadSynchronous();
	TSubclassOf<ALrAIControllerBase> BossClass = Settings->BossAIControllerClass.LoadSynchronous();

	if (NormalClass)
	{
		NormalAIPool.PreCreateControllers(World, NormalClass);
	}
	if (BossClass)
	{
		BossAIPool.PreCreateControllers(World, BossClass);
	}
}

ALrAIControllerBase* ULrAIManagerSubsystem::SpawnNormalAI(ALrEnemyPawn* Enemy)
{
	const ULrAISettings* Settings = GetDefault<ULrAISettings>();
	if (!Settings)
	{
		return nullptr;
	}

	TSubclassOf<ALrAIControllerBase> ControllerClass = Settings->NormalAIControllerClass.LoadSynchronous();
	return AcquireController(Enemy, ControllerClass, NormalAIPool);
}

ALrAIControllerBase* ULrAIManagerSubsystem::SpawnBossAI(ALrEnemyPawn* Enemy)
{
	const ULrAISettings* Settings = GetDefault<ULrAISettings>();
	if (!Settings)
	{
		return nullptr;
	}
	TSubclassOf<ALrAIControllerBase> ControllerClass = Settings->BossAIControllerClass.LoadSynchronous();
	return AcquireController(Enemy, ControllerClass, BossAIPool);
}

void ULrAIManagerSubsystem::SwitchToBossAI(ALrEnemyPawn* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	ALrAIControllerBase* OldController = Cast<ALrAIControllerBase>(Enemy->GetController());

	if (OldController)
	{
		OldController->ShutdownAI();
		OldController->UnPossess();
		RecycleController(OldController);
	}

	SpawnBossAI(Enemy);
}

void ULrAIManagerSubsystem::SwitchToNormalAI(ALrEnemyPawn* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	ALrAIControllerBase* OldController = Cast<ALrAIControllerBase>(Enemy->GetController());

	if (OldController)
	{
		OldController->ShutdownAI();
		OldController->UnPossess();
		RecycleController(OldController);
	}

	SpawnNormalAI(Enemy);
}

ALrAIControllerBase* ULrAIManagerSubsystem::AcquireController(ALrEnemyPawn* Enemy, TSubclassOf<ALrAIControllerBase> ControllerClass, FLrAIControllerPool& Pool)
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

	ALrAIControllerBase* Controller = Pool.GetFromPool(World, ControllerClass);
	if (!Controller)
	{
		return nullptr;
	}

	// 设置位置并 Possess
	// AIController 的位置/旋转不重要，Possess 后由 Pawn 决定
	// 对象池回收时 UnPossess 了，重新 Possess 即可恢复所有状态
	Controller->Possess(Enemy);

	return Controller;
}

void ULrAIManagerSubsystem::ReleaseController(ALrAIControllerBase* Controller, FLrAIControllerPool& Pool)
{
	if (!Controller)
	{
		return;
	}

	Pool.ReturnToPool(Controller);
}

void ULrAIManagerSubsystem::RecycleController(ALrAIControllerBase* Controller)
{
	if (!Controller)
	{
		return;
	}

	// 根据控制器类型回收到对应池
	if (Controller->IsA<ALrBossAIController>())
	{
		ReleaseController(Controller, BossAIPool);
	}
	else
	{
		ReleaseController(Controller, NormalAIPool);
	}
}
