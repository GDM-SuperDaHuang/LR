// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LrCorpseConfigDA.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FLrCorpseConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Corpse")
	UPhysicsAsset* CustomPhysicsAsset = nullptr; // 某些怪可能需要特殊的物理资产

	UPROPERTY(EditDefaultsOnly, Category = "Corpse")
	UAnimMontage* CorpseMontage = nullptr;
	// UPROPERTY(EditDefaultsOnly, Category = "Corpse")
	// USkeletalMeshComponent* CorpseMesh = nullptr;;

	UPROPERTY(EditDefaultsOnly, Category = "Corpse")
	float LifeDuration = 5.0f; // 普通怪躺5秒，Boss躺30秒

	UPROPERTY(EditDefaultsOnly, Category = "Corpse")
	float CollisionScale = 1.0f; // 尸体体型缩放

	UPROPERTY(EditDefaultsOnly, Category = "Corpse")
	uint32 PawnType = 0; //生物类型
};

/**
 * 
 */
UCLASS()
class LR_API ULrCorpseConfigDA : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FLrCorpseConfig> LrCorpseConfig;


	const FLrCorpseConfig* FindCorpseConfigByPawnType(const uint32 PawnType) const;
};
