// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "GenericTeamAgentInterface.h"
#include "MoverSimulationTypes.h"
#include "Component/Combat/LrCombatComponentBase.h"
#include "GameFramework/Pawn.h"
#include "Interface/LrCombatInterface.h"
#include "LrPawnBase.generated.h"

class ULrASC;
class ULrAnimationComponent;
class ULrMoverComponent;
class UMotionWarpingComponent;
class UCharacterMoverComponent;
// class ULrMoverComponent;
class ULrNavMovementComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnASCRegistered, ULrASC*);

UCLASS()
class LR_API ALrPawnBase : public APawn, public IMoverInputProducerInterface, public IAbilitySystemInterface, public IGenericTeamAgentInterface, public ILrCombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ALrPawnBase();

	UPROPERTY(EditAnywhere)
	uint16 PawnType; //生物类型
	UPROPERTY(EditDefaultsOnly, Category="Team")
	uint8 TeamID = 1;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual ULrCombatComponentBase* GetCombatComponent() const override;
	virtual uint8 GetTeamID() const override;
	virtual bool IsDead() const override;
	virtual ULrASC* GetASC() const override;
	virtual ULrAS* GetAS() const override;
	virtual uint8 GetClassID() const;
	virtual void ToDie(const FLrDieParameters& LrDieConfig) override;
	/** 战斗组件 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULrCombatComponentBase> LrCombatComponent;

	/** 身体骨骼 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<USkeletalMeshComponent> LrSkeletalMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PawnInfo")
	TObjectPtr<UMotionWarpingComponent> LrMotionWarpingComponent;

	/** 角色移动组件（Mover系统的核心） */
	UPROPERTY(Category = Movement, VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULrMoverComponent> LrMoverComponent;

	/** 武器 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UChildActorComponent> EquippedWeaponComponent;
	/** 武器 */

	FOnASCRegistered OnASCRegistered;

	/** 属性UI组件 */
	// UPROPERTY(VisibleAnywhere)
	// TObjectPtr<ULrASComponent> LrASComponent;

	/** 动画相关组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ULrAnimationComponent> LrAnimationComponent;

	/** 导航组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="LrNavMoverComponent")
	TObjectPtr<ULrNavMovementComponent> LrNavMoverComponent;

protected:
	/** mover写入网络时 */
	UFUNCTION()
	void HandleMoverFinalized(const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState);
	/** mover模式切换时 */
	UFUNCTION()
	void HandleOnMovementModeChanged(const FName& PreviousMovementModeName, const FName& NewMovementModeName);
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
	// virtual void OnProduceInput(float DeltaMs, FMoverInputCmdContext& InputCmdResult);

	UPROPERTY()
	TObjectPtr<ULrASC> LrASC;
	UPROPERTY()
	TObjectPtr<ULrAS> LrAS;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FGenericTeamId GetGenericTeamId() const override;

	void UpdateMove(FVector Input);
	void UpdatePressedJump(bool Input);

	// 地面摩檫力物理
	void CheckFloorPhysics(float& OutFrictionMult);
	void InitAS() const;

public:
	bool bIsJumpJustPressed = false; // 本帧刚刚按下
private:
	/** 帧更新，最后一次非零移动输入（用于维持朝向） */
	// FVector2D CachedMoveInput = FVector2D::ZeroVector; // Movement input (intent or velocity) the last time we had one that wasn't zero
	bool bIsJumpPressed = false; // 当前处于按住状态
	FVector CachedMoveInput = FVector::ZeroVector; // Movement input (intent or velocity) the last time we had one that wasn't zero

	FVector LastVelocity = FVector::ZeroVector; //当前速度，只有>0.5,或者小于0.5的瞬间才触发委托
	FCharacterDefaultInputs CharacterDefaultInputsPre;
	FMoverDataCollection InputDataCollection;
	/** 最后一次非零移动输入（用于维持朝向） */


protected:
	// 动态材质实例
	// 使用 TArray 存储所有材质槽位的动态材质，UPROPERTY() 必须加，防止被 GC 回收
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> DissolveMIDs;

	FTimerHandle DissolveTimerHandle;
	float DissolveStartTime;
	float DissolveDuration = 3.f;

	void UpdateDissolveProgress();
};
