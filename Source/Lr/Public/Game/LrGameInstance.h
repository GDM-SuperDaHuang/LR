// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Player/Input/LrInputComponent.h"
#include "LrGameInstance.generated.h"

class ULrSaveGame;
struct FGameplayTag;
/**
 * 
 */
UCLASS()
class LR_API ULrGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	// 加载
	void LoadInputSave();

	// 保存
	void SaveInput();

	// 改键
	void RebindKey(FGameplayTag InputTag, FKey NewKey);

	//ui调用 GI->RebindKey(...);
	void RefreshInputBind(UInputMappingContext* MappingContext, const ULrInputConfigDA* InputConfig) const;

public:
	// 输入存档
	UPROPERTY()
	TObjectPtr<ULrSaveGame> InputSaveGame;
};
