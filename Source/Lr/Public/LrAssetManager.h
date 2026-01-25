// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "LrAssetManager.generated.h"

/**
 *需要在配置加上
* [/Script/Engine.Engine]
AssetManagerClassName=/Script/Lr.LrAssetManager
 */
UCLASS()
class LR_API ULrAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	static ULrAssetManager& Get();
protected:
	virtual void StartInitialLoading() override;
};
