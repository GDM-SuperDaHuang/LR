// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "MoverSimulationTypes.h"
#include "GameFramework/Pawn.h"
#include "LrPawnBase.generated.h"

class ULrAnimationComponent;
class ULrASComponent;
class ULrMoverComponent;
class UMotionWarpingComponent;
class UAttributeSet;
class UAbilitySystemComponent;
class UCharacterMoverComponent;
// class ULrMoverComponent;
class ULrNavMovementComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnASCRegistered, UAbilitySystemComponent*);

UCLASS()
class LR_API ALrPawnBase : public APawn, public IMoverInputProducerInterface, public IAbilitySystemInterface,public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ALrPawnBase();

	// virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 身体骨骼 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<USkeletalMeshComponent> LrSkeletalMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<UMotionWarpingComponent> LrMotionWarpingComponent;

	/** 角色移动组件（Mover系统的核心） */
	UPROPERTY(Category = Movement, VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULrMoverComponent> CharacterMotionComponent;
	
	/** 武器 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UChildActorComponent> EquippedWeaponComponent;
	/** 武器 */

	FOnASCRegistered OnASCRegistered;

	/** 属性UI组件 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULrASComponent> LrASComponent;

	/** 动画相关组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ULrAnimationComponent> LrAnimationComponent;

	/** 导航组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="LrNavMoverComponent")
	TObjectPtr<ULrNavMovementComponent> LrNavMoverComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/**
	 * 输入生产入口点，不要重写！
	 * 这是IMoverInputProducerInterface的核心实现，Mover系统每模拟帧调用一次
	 * 要扩展功能，请重写OnProduceInput（C++）或实现"Produce Input"蓝图事件
	 */
	virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;

	/**
	 * 蓝图实现事件，为下一模拟帧生成输入
	 * 在蓝图中实现"Produce Input"事件来扩展输入逻辑
	 * @param DeltaMs 距离上次模拟的毫秒数
	 * @param InputCmd 输入命令，可以修改后传出
	 */
	virtual void OnProduceInput(float DeltaMs, FMoverInputCmdContext& InputCmdResult);

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> LrASC;
	UPROPERTY()
	TObjectPtr<UAttributeSet> LrAS;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	virtual FGenericTeamId GetGenericTeamId() const override;
	
	UPROPERTY(EditDefaultsOnly, Category="Team")
	uint8 TeamID = 1;
	
	void UpdateMove(FVector Input);
	void UpdatePressedJump(bool Input);

	// 地面摩檫力物理
	void CheckFloorPhysics(float& OutFrictionMult);
public:
	bool bIsJumpJustPressed = false; // 本帧刚刚按下
private:
	/** 帧更新，最后一次非零移动输入（用于维持朝向） */
	// FVector2D CachedMoveInput = FVector2D::ZeroVector; // Movement input (intent or velocity) the last time we had one that wasn't zero
	bool bIsJumpPressed = false; // 当前处于按住状态
	FVector CachedMoveInput = FVector::ZeroVector; // Movement input (intent or velocity) the last time we had one that wasn't zero

	FCharacterDefaultInputs CharacterDefaultInputsPre;
	FMoverDataCollection InputDataCollection;
	/** 最后一次非零移动输入（用于维持朝向） */
};
