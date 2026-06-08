#pragma once
#include "EngineUtils.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "AI/ST/LrSTComponent.h"
#include "Pawn/LrEnemyPawn.h"
#include "FLrStateTreeEvaluator.generated.h"


USTRUCT(BlueprintType)
struct FLrEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Settings")
	float SearchRadius = 1000.f;
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
		Linker.LinkExternalData(TargetHandle);
		return true;
	}

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override
	{
		ALrEnemyPawn& Data = Context.GetExternalData(TargetHandle);
		// if (!Data.SelfPawn)
		// 	return;
		//
		// // 找最近敌人示例
		// UWorld* World = Data.SelfPawn->GetWorld();
		// if (!World) return;
		//
		// AActor* Closest = nullptr;
		// float ClosestDist = MAX_flt;
		//
		// for (TActorIterator<APawn> It(World); It; ++It)
		// {
		// 	APawn* Pawn = *It;
		// 	if (Pawn == Data.SelfPawn) continue;
		//
		// 	float Dist = FVector::Dist(Pawn->GetActorLocation(), Data.SelfPawn->GetActorLocation());
		// 	if (Dist < ClosestDist)
		// 	{
		// 		ClosestDist = Dist;
		// 		Closest = Pawn;
		// 	}
		// }
		//
		// Data.TargetActor = Closest;
	}
protected:
	TStateTreeExternalDataHandle<ALrEnemyPawn> TargetHandle;

};
