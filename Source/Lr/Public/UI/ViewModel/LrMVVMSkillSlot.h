// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "LrMVVMSkillSlot.generated.h"

class ULrGABase;
class UTexture2D;

UCLASS()
class LR_API ULrMVVMSkillSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(ULrGABase* InAbility, AActor* InOwnerActor, int32 InInputID);

	void Tick(float DeltaTime);

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Skill")
	TObjectPtr<UTexture2D> SkillIcon;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Skill")
	FText CooldownText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Skill")
	float CooldownRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Skill")
	float CooldownTotal = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Skill")
	float CooldownPercent = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Skill")
	bool bInCooldown = false;

private:
	UPROPERTY()
	TObjectPtr<ULrGABase> Ability;

	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;

	int32 InputID = -1;
};