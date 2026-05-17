// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "UObject/Object.h"
#include "LrMovementSettings.generated.h"

/**
 * 
 */
UCLASS()
class LR_API ULrMovementSettings : public UObject, public IMovementSettingsInterface
{
	GENERATED_BODY()
public:
	ULrMovementSettings();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float Mass = 80.0f;

	// --- ГРАВИТАЦИЯ ---
	// 1.0 = Земля (медленно для игр). 
	// 2.0 - 3.0 = Шутер/Экшен (резкое падение).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air")
	float GravityScale = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk")
	float MaxWalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk")
	float SprintSpeedMult = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk")
	float BrakingDeceleration = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk")
	float MaxWalkForce = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk")
	float GroundFriction = 8.0f;

	// 启动/大角度转向时的速度惩罚：0 = 无惩罚（瞬间满速），1 = 最大惩罚（需要完整加速时间）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StartSpeedPenalty = 0.5f;

	// 从惩罚状态恢复到满速所需的时间（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk")
	float AccelerationRampUpTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float JumpImpulseForce = 120000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouch")
	float CrouchSpeedMult = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air")
	float AirDragCoef = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air")
	float AirControlForce = 500.0f;

	virtual FString GetDisplayName() const override { return TEXT("Realistic Physics"); }
};
