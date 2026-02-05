// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "LrGAListDA.generated.h"


class UNiagaraSystem;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FLrGAConfig
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> GAClass; //技能

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag GATag = FGameplayTag(); // 技能本身

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag(); // 触发技的标签。

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<UAnimMontage*> MontageList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ImpactSound = nullptr;
};

// 特效配置
USTRUCT(BlueprintType)
struct FLrNSConfig
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag NSTag = FGameplayTag(); // 特效的标签。

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> NS;
};

// 武器配置
USTRUCT(BlueprintType)
struct FLrWeaponConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	int32 WeaponID = -1;

	/** 武器类 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ALrWeaponBase> WeaponClass;

	UPROPERTY(EditDefaultsOnly)
	TArray<USkeletalMesh*> WeaponSMList;
};

/**
 * 
 */
UCLASS()
class LR_API ULrGAListDA : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FLrGAConfig> GAConfigList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FLrNSConfig> NSConfigList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FLrWeaponConfig> LrWeaponConfigList;

	const FLrGAConfig* FindGAByTag(const FGameplayTag& GATag) const;
	const FLrNSConfig* FindNSByTag(const FGameplayTag& Tag) const;
	const FLrWeaponConfig* FindWeaponByID(const int32 WeaponID) const;
};
