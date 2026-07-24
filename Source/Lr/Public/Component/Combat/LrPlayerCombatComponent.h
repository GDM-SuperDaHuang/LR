// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LrCombatComponentBase.h"
#include "Engine/HitResult.h"
#include "LrPlayerCombatComponent.generated.h"

USTRUCT(BlueprintType)
struct FLrAimData
{
	GENERATED_BODY()

	/** 是否命中 */
	UPROPERTY(BlueprintReadOnly)
	bool bHit = false;

	/** 命中信息 */
	UPROPERTY(BlueprintReadOnly)
	FHitResult HitResult;

	/** Camera位置 */
	UPROPERTY(BlueprintReadOnly)
	FVector ViewLocation = FVector::ZeroVector;

	/** Camera方向 */
	UPROPERTY(BlueprintReadOnly)
	FVector ViewDirection = FVector::ForwardVector;

	/** 最终瞄准点 */
	UPROPERTY(BlueprintReadOnly)
	FVector AimPoint = FVector::ZeroVector;
};
class ALrPawnBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LR_API ULrPlayerCombatComponent : public ULrCombatComponentBase
{
	GENERATED_BODY()

public:
	ULrPlayerCombatComponent();

	// 新增：手动选择一个目标
	virtual TWeakObjectPtr<ALrPawnBase> GetNearestPawnToCursor();
	
	// 从准心获取一个位置
	virtual void GetAimPoint(FLrAimData& OutAimData);
	
};
