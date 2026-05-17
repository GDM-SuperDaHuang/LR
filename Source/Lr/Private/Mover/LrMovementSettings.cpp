// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover/LrMovementSettings.h"

ULrMovementSettings::ULrMovementSettings()
{
	Mass = 80.0f;

	// Увеличиваем гравитацию в 2 раза, чтобы он не летал как пушинка
	GravityScale = 2.0f;

	MaxWalkSpeed = 600.0f;
	MaxWalkForce = 2048.0f;
	BrakingDeceleration = 2048.0f;
	GroundFriction = 8.0f;

	JumpImpulseForce = 120000.0f;
	SprintSpeedMult = 1.5f;
	CrouchSpeedMult = 0.5f;
	AirDragCoef = 0.1f;
	AirControlForce = 500.0f;
}
