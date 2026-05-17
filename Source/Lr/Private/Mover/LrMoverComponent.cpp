// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/LrMoverComponent.h"

#include "Mover/LrAllModes.h"
#include "Mover/LrMovementSettings.h"
#include "Mover/Air/LrAirMovementMode.h"
#include "Mover/Blink/LrBlinkMovementMode.h"
#include "Mover/Walk/LrWalkMovementMode.h"


ULrMoverComponent::ULrMoverComponent()
{
	// Регистрируем режимы
	MovementModes.Add(LrAllModes::Walk, CreateDefaultSubobject<ULrWalkMovementMode>(TEXT("LrWalkMovementMode")));
	MovementModes.Add(LrAllModes::Air, CreateDefaultSubobject<ULrAirMovementMode>(TEXT("LrAirMovementMode")));
	MovementModes.Add(LrAllModes::Blink, CreateDefaultSubobject<ULrBlinkMovementMode>(TEXT("LrBlinkMovementMode")));

	// Стартовый режим
	StartingMovementMode = LrAllModes::Air;

	// Создаем настройки, чтобы они были видны в Blueprint
	RealisticSettings = CreateDefaultSubobject<ULrMovementSettings>(TEXT("RealisticSettings"));
}
