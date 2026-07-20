// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LrSkillWidget.generated.h"

class ULrMVVMSkillSlot;

UCLASS()
class LR_API ULrSkillWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetViewModel(FName ViewModelName, ULrMVVMSkillSlot* InViewModel);

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	TObjectPtr<ULrMVVMSkillSlot> ViewModel;
};