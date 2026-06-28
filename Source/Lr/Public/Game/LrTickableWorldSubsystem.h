// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LrTickableWorldSubsystem.generated.h"

class ALrLightning;
class ALrPawnBase;
struct FLrProjectileConfigRow;
class ALrProjectile;
class ULrWorldBarWidget;
/**
 * 
 */
UCLASS()
class LR_API ULrTickableWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual TStatId GetStatId() const override;
	virtual void Tick(float DeltaTime) override;
	//////////////////////////////////////////////////////////////////////////
	// UI
	void RegisterActiveBar(ULrWorldBarWidget* Bar);

private:
	TArray<TWeakObjectPtr<ULrWorldBarWidget>> ActiveBars;
	//////////////////////////////////////////////////////////////////////////
public:
	//////////////////////////////////////////////////////////////////////////
	// 闪电特效对象池
	/** 闪电 Actor 蓝图模板（ALrLightning 子类） */
	UPROPERTY()
	TMap<ALrPawnBase*, ALrLightning*> LrLightningPool;
	ALrLightning* RegisterLightning(ALrPawnBase* LrPawnBase);
	void RemoveLightning(const ALrPawnBase* ALrPawnBas);
	ALrLightning* GetLightning( ALrPawnBase* ALrPawnBas);
	//////////////////////////////////////////////////////////////////////////
};
