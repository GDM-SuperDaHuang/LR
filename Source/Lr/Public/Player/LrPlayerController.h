// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LrPlayerController.generated.h"

class ULrDataAsset;
struct FGameplayTag;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class LR_API ALrPlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
private:
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<ULrDataAsset> InputConfig;


	/** 移动相关 */
	// 增强输入资源
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> LrInputMappingContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> DashAction;

	void Move(const FInputActionValue& InputActionValue) ;
	void JumpPress(const FInputActionValue& InputActionValue) ;
	void DashPress(const FInputActionValue& InputActionValue) ;

	/** 移动相关 */


	/** 技能释放相关 */
	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
	/** 技能释放相关 */




};
