// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UIController/LrUIController.h"

#include "Component/LrASComponent.h"
#include "UI/ViewModel/LrMVVMVBar.h"

void ULrUIController::Init(ULrASComponent* InASComponent)
{
	ASComponent = InASComponent;

	HPVM = NewObject<ULrMVVMVBar>(this);
	MPVM = NewObject<ULrMVVMVBar>(this);

	InASComponent->OnHealthChanged.AddUObject(this,&ULrUIController::OnHPChanged);
	InASComponent->OnManaChanged.AddUObject(this,&ULrUIController::OnMPChanged);

	OnHPChanged(InASComponent->GetHealth(),InASComponent->GetMaxHealth());
	OnMPChanged(InASComponent->GetMana(),InASComponent->GetMaxMana());
}

void ULrUIController::Tick(float DeltaTime)
{
	if (HPVM)
	{
		HPVM->Tick(DeltaTime);
	}

	if (MPVM)
	{
		MPVM->Tick(DeltaTime);
	}
}

void ULrUIController::OnHPChanged(float Current, float Max)
{
	HPVM->SetValue(Current, Max);
}

void ULrUIController::OnMPChanged(float Current, float Max)
{
	MPVM->SetValue(Current, Max);
}
