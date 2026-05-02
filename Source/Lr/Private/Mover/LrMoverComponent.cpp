// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/LrMoverComponent.h"

#include "Mover/RealisticModes.h"
#include "Mover/RealisticMovementSettings.h"
#include "Mover/Air/LrAirMovementMode.h"
#include "Mover/Walk/LrWalkMovementMode.h"


ULrMoverComponent::ULrMoverComponent()
{
	// Регистрируем режимы
	MovementModes.Add(RealisticModes::Walk, CreateDefaultSubobject<ULrWalkMovementMode>(TEXT("LrWalkMovementMode")));
	MovementModes.Add(RealisticModes::Air, CreateDefaultSubobject<ULrAirMovementMode>(TEXT("ULrAirMovementMode")));

	// Стартовый режим
	StartingMovementMode = RealisticModes::Air;

	// Создаем настройки, чтобы они были видны в Blueprint
	RealisticSettings = CreateDefaultSubobject<URealisticMovementSettings>(TEXT("RealisticSettings"));

}
