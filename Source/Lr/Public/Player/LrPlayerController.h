// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LrPlayerController.generated.h"

class ALrPawnBase;
class ULrSaveGame;
class ULrASC;
struct FGameplayTag;
struct FInputActionValue;
class ULrInputConfigDA;
class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class LR_API ALrPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALrPlayerController();
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;
	// virtual void CreateInputComponent(TSubclassOf<UInputComponent> InputComponentToCreate) override;
private:
	UPROPERTY()
	TObjectPtr<ULrASC> LrASC;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<ULrInputConfigDA> InputConfig;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> LrIMC;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	void AbilityInputTagPressed0(int32 InputId);
	void AbilityInputTagReleased0(int32 InputId);
	void AbilityInputTagHeld0(int32 InputId);

	void Move(const FInputActionValue& InputActionValue);
	void MoveCompleted(const FInputActionValue& InputActionValue);
	void Look(const FInputActionValue& InputActionValue);
	void Jump() const;

public:
	UFUNCTION()
	ALrPawnBase* GetNearestPawnToCursor(float MaxScreenDistance = 120.f);
	void UpdateHoverTarget();

	UPROPERTY()
	ALrPawnBase* CurrentSelectedPawn;

	bool InhibitoryInput = false;


	//相机
private:
	void UpdateCamera(float DeltaSeconds);

private:

	/** 当前相机Yaw */
	float CurrentCameraYaw = 0.f;

	/** 目标Yaw */
	float TargetCameraYaw = 0.f;

public:

	UPROPERTY(EditAnywhere, Category="Camera")
	float CameraPitch = -55.f;

	UPROPERTY(EditAnywhere, Category="Camera")
	float CameraRotateSpeed = 220.f;

	/** 鼠标控制摄像机Yaw的灵敏度（度/每单位输入） */
	UPROPERTY(EditAnywhere, Category="Camera")
	float CameraYawSpeed = 0.5f;

	/** 鼠标控制摄像机Pitch的灵敏度（度/每单位输入） */
	UPROPERTY(EditAnywhere, Category="Camera")
	float CameraPitchSpeed = 0.3f;

	/** 摄像机Pitch最小值（防止贴地） */
	UPROPERTY(EditAnywhere, Category="Camera")
	float MinCameraPitch = -75.f;

	/** 摄像机Pitch最大值（防止俯视太平） */
	UPROPERTY(EditAnywhere, Category="Camera")
	float MaxCameraPitch = -10.f;
};
