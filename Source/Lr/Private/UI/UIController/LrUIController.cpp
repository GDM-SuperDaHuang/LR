// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UIController/LrUIController.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "ASC/LrASC.h"
#include "Tags/LrGameplayTags.h"
#include "ASC/GA/LrGABase.h"
#include "Pawn/LrPawnBase.h"
#include "UI/ViewModel/LrMVVMVBar.h"
#include "UI/ViewModel/LrMVVMSkillSlot.h"

void ULrUIController::Init()
{
	// ASComponent = InASComponent;

	HPVM = NewObject<ULrMVVMVBar>(this);
	MPVM = NewObject<ULrMVVMVBar>(this);

	// InASComponent->OnHealthChanged.AddUObject(this, &ULrUIController::OnHPChanged);
	// InASComponent->OnManaChanged.AddUObject(this, &ULrUIController::OnMPChanged);

	// OnHPChanged(InASComponent->GetHealth(), InASComponent->GetMaxHealth());
	// OnMPChanged(InASComponent->GetMana(), InASComponent->GetMaxMana());
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

	for (ULrMVVMSkillSlot* SkillSlotVM : SkillSlotVMs)
	{
		if (SkillSlotVM)
		{
			SkillSlotVM->Tick(DeltaTime);
		}
	}
}

void ULrUIController::OnASChanged(FGameplayTag ASTag, float Current, float Max)
{
	FLrGameplayTags LrTags = FLrGameplayTags::Get();
	if (ASTag == LrTags.As_HP)
	{
		OnHPChanged(Current, Max);
	}
	else if (ASTag == LrTags.As_MP)
	{
		OnMPChanged(Current, Max);
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

void ULrUIController::InitSkillSlots(ALrPawnBase* OwnerPawn)
{
	if (!OwnerPawn)
	{
		return;
	}

	ULrASC* ASC = Cast<ULrASC>(OwnerPawn->GetAbilitySystemComponent());
	if (!ASC)
	{
		return;
	}

	SkillSlotVMs.Empty();

	FScopedAbilityListLock ActiveScopeLock(*ASC);
	const TArray<FGameplayAbilitySpec>& ActivatableAbilities = ASC->GetActivatableAbilities();

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities)
	{
		ULrGABase* GameplayAbility = Cast<ULrGABase>(Spec.Ability);
		if (!GameplayAbility)
		{
			continue;
		}

		ULrMVVMSkillSlot* SkillSlotVM = NewObject<ULrMVVMSkillSlot>(this);
		SkillSlotVM->Initialize(GameplayAbility, OwnerPawn, Spec.InputID);
		SkillSlotVMs.Add(SkillSlotVM);
	}
}
