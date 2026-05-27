// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Component/LrWorldWidgetComponent.h"

ULrWorldWidgetComponent::ULrWorldWidgetComponent()
{
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawAtDesiredSize(true);
	BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetTwoSided(true);

	SetCastShadow(false);

	SetReceivesDecals(false);

	// SetWindowVisibility(EWindowVisibility::SelfHitTestInvisible);

	bApplyGammaCorrection = false;

	SetPivot(FVector2D(0.5f, 0.5f));

	PrimaryComponentTick.bCanEverTick = false;
}

// void ULrWorldWidgetComponent::BeginPlay()
// {
// 	Super::BeginPlay();
//
// 	if (LrWidgetClass)
// 	{
// 		this->SetWidgetClass(LrWidgetClass);
// 	}
// 	this->InitWidget();
// }
