// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "LrPS.generated.h"

class ULrAS;
class ULrASC;
/**
 * 
 */
UCLASS()
class LR_API ALrPS : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ALrPS();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FORCEINLINE ULrAS* GetAttributeSet() const { return LrAS; }




protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULrASC> LrASC;
	UPROPERTY()
	TObjectPtr<ULrAS> LrAS;
};
