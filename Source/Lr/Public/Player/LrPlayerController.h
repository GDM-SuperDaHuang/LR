// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LrPlayerController.generated.h"

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
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	// virtual void CreateInputComponent(TSubclassOf<UInputComponent> InputComponentToCreate) override;
private:
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<ULrInputConfigDA> InputConfig;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ShiftAction;


	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);


	
	void Move(const FInputActionValue& InputActionValue);
	void MoveCompleted(const FInputActionValue& InputActionValue);
	void Jump(const FInputActionValue& InputActionValue);

};
