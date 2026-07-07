#pragma once

#include "DrawDebugHelpers.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Component/LrPatrolRouteComponent.h"
#include "Engine/World.h"
#include "Game/LrTickableWorldSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Pawn/LrEnemyPawn.h"
#include "Pawn/LrHeroPawn.h"
#include "FLrStateTreeEvaluator.generated.h"


USTRUCT(BlueprintType)
struct FLrEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="AI")
	float DetectRadius = 500.f;

	UPROPERTY(EditAnywhere, Category="AI")
	float UpdateInterval = 0.2f; //更新频率

	UPROPERTY()
	float ElapsedTime = 0.f;

	UPROPERTY(EditAnywhere, Category="AI")
	bool IsJumpScare = false; //是否会吓人
};

USTRUCT(BlueprintType)
struct FLrStateTreeEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	//上下文内容
	using FInstanceDataType = FLrEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool Link(FStateTreeLinker& Linker) override
	{
		Linker.LinkExternalData(EnemyHandle);
		return true;
	}

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override
	{
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		InstanceData.ElapsedTime += DeltaTime;
		if (InstanceData.ElapsedTime < InstanceData.UpdateInterval)
		{
			return;
		}
		InstanceData.ElapsedTime = 0.f;
		ALrEnemyPawn& Enemy = Context.GetExternalData(EnemyHandle);
		UWorld* World = Enemy.GetWorld();
		if (!World)
		{
			return;
		}

		//获取玩家
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (!PC)
			{
				continue;
			}
			TObjectPtr<APawn> Pawn = PC->GetPawn();
			if (!Pawn)
			{
				continue;
			}
			ALrHeroPawn* Player = Cast<ALrHeroPawn>(PC->GetPawn());
			if (!Player)
			{
				continue;
			}

			// 如果被攻击 吸引仇恨
			// Enemy.GetPatrolRoute()->TargetActor = Player;
			FVector EnemyLocation = Enemy.GetActorLocation();
			FVector PlayerLocation = Pawn->GetActorLocation();
			const float DistSq = FVector::DistSquared(EnemyLocation, PlayerLocation); //距离平方
			//-----------------------------------
			// 1. 距离检测
			//-----------------------------------
			if (DistSq > FMath::Square(InstanceData.DetectRadius))
			{
				continue;
			}

			//可能会吓人
			if (InstanceData.IsJumpScare)
			{
				Enemy.GetPatrolRoute()->TargetActor = Player;

				if (ULrTickableWorldSubsystem* Subsystem = World->GetSubsystem<ULrTickableWorldSubsystem>())
				{
					Subsystem->PlayJumpScare(Player, &Enemy);
				}
				continue;
			}


			//-----------------------------------
			// 2. 120°视野检测
			//-----------------------------------
			FVector ToPlayer = PlayerLocation - EnemyLocation;
			ToPlayer.Z = 0.f;
			if (!ToPlayer.Normalize())
			{
				continue;
			}

			FVector Forward = Enemy.GetActorForwardVector();
			Forward.Z = 0.f;
			Forward.Normalize();
			const float Dot = FVector::DotProduct(Forward, ToPlayer);
			// 120°
			// 左60°~右60°
			// cos(60)=0.5
			if (Dot < 0.5f)
			{
				// Enemy.SetTargetActor(nullptr);
				return;
			}

			//-----------------------------------
			// 3. LineTrace遮挡检测
			//-----------------------------------
			FVector TraceStart = EnemyLocation + FVector(0, 0, 50); //抬高50,再发射
			FVector TraceEnd = PlayerLocation + FVector(0, 0, 50);
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(&Enemy);
			bool bBlockingHit = World->LineTraceSingleByChannel(
				Hit,
				TraceStart,
				TraceEnd,
				ECC_Visibility,
				Params);

			// 调试线
			DrawDebugLine(
				World,
				TraceStart,
				TraceEnd,
				FColor::Green,
				false,
				0.2f,
				0,
				2.f);
			AActor* TestActor = Hit.GetActor();
			if (!bBlockingHit) //什么也没击中
			{
				// Enemy.SetTargetActor(Player);
				continue;
			}
			if (Hit.GetActor() != Player)
			{
				// Enemy.SetTargetActor(Player);
				continue;
			}

			Enemy.GetPatrolRoute()->TargetActor = Player;
		}
	}

protected:
	TStateTreeExternalDataHandle<ALrEnemyPawn> EnemyHandle;
};
