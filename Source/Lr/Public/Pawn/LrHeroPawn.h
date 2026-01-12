// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pawn/LrPawnBase.h"
#include "LrHeroPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
/**
 * 
 */
UCLASS()
class LR_API ALrHeroPawn : public ALrPawnBase
{
	GENERATED_BODY()

public:
	ALrHeroPawn();

	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;
protected:
	/** 碰撞体（NavAgent & Mover 都依赖它） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<UCapsuleComponent> LrCapsuleComponent;

	/** 骨骼 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<USkeletalMeshComponent> LrSkeletalMeshComponent;

	/** 相机 弹簧臂*/
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComponent;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;
	/** 相机 弹簧臂*/


	
};
