// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LrTickableWorldSubsystem.generated.h"

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
	void RegisterActiveBar(ULrWorldBarWidget* Bar);
private:

	TArray<TWeakObjectPtr<ULrWorldBarWidget>> ActiveBars;
};
