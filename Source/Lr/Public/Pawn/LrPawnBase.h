// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LrPawnBase.generated.h"

// USTRUCT()
// struct FActionStatus 
// {
//     GENERATED_BODY()

//     UPROPERTY()
//     FVector2D CachedMoveInput = FVector2D::ZeroVector;
//     bool bJumpInput;
//     bool bDashInput;
//     uint8 bJumpPressed : 1;
//     uint8 bDashPressed : 1;
//     FActionStatus()
//         : bJumpPressed(false)
//         , bDashPressed(false)
//     {}
// };


class UMoverComponent;

UCLASS()
class LR_API ALrPawnBase : public APawn, public IMoverInputProducerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ALrPawnBase();

	/** 移动更新相关 */
	void UpdateMove(const FVector2D Input);
	void SetJump(bool Input);
	void SetDash(bool Input);
	/** 移动更新相关 */
protected: 
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(Category = Movement, VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterMoverComponent> CharacterMotionComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** 移动更新相关 */
	FVector2D CachedMoveInput = FVector2D::ZeroVector;;
    bool bJumpPressed;
    bool bDashPressed;
	/** 移动更新相关 */


	
};

// AMoverExamplesCharacter