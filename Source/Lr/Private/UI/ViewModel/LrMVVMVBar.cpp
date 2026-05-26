// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/LrMVVMVBar.h"

void ULrMVVMVBar::SetValue(float InCurrent, float InMax)
{
	CurrentValue = InCurrent;
	MaxValue = InMax;
	UE_LOG(LogTemp, Warning, TEXT("MVVM: %p"), this);
	
	const float NewPercent = MaxValue <= 0.f ? 0.f : CurrentValue / MaxValue;

	UE_MVVM_SET_PROPERTY_VALUE(Percent, NewPercent); //修改值+只会通知 UI

	TargetPercent = NewPercent;

	ValueText = FText::Format(
		NSLOCTEXT("Bar", "HPFormat", "{0} / {1}"),
		FText::AsNumber(FMath::RoundToInt(CurrentValue)),
		FText::AsNumber(FMath::RoundToInt(MaxValue))
	);

	// UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentValue);
	// UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MaxValue);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ValueText); //不会修改值,只会通知 UI
}

void ULrMVVMVBar::Tick(float DeltaTime)
{
	const float NewGhost = FMath::FInterpTo(GhostPercent, TargetPercent, DeltaTime, InterpSpeed);
	UE_MVVM_SET_PROPERTY_VALUE(GhostPercent, NewGhost);
}
