// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASC/GE/LrGEContext.h"
#include "Component/Combat/LrCombatComponentBase.h"
#include "GameFramework/Actor.h"
#include "LrLightning.generated.h"

class UNiagaraSystem;
class ALrPawnBase;
class UNiagaraComponent;

USTRUCT(BlueprintType)
struct FLrLightningBeam
{
	GENERATED_BODY()

	/** Niagara Beam组件 */
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> NiagaraComponent = nullptr;

	/** 是否正在使用 */
	UPROPERTY()
	bool bActive = false;

	/** 起点Actor */
	UPROPERTY()
	TWeakObjectPtr<ALrPawnBase> StartActor = nullptr;

	/** 终点Actor */
	UPROPERTY()
	TWeakObjectPtr<ALrPawnBase> EndActor = nullptr;
};

USTRUCT()
struct FLrTargetInfo
{
	GENERATED_BODY()

	/** 链目标 */
	UPROPERTY()
	TWeakObjectPtr<ALrPawnBase> Target;

	UPROPERTY()
	float ElapsedTime = 0.f;
};

UCLASS()
class LR_API ALrLightning : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALrLightning();
	void Attack(ALrPawnBase* InCaster);

	/** 初始化，设置施法者与目标列表 */
	void Init(ALrPawnBase* InCaster);

	/** 停止攻击 */
	void Stop();

	/** 清除所有连接 */
	void Clear();

	/** 添加目标 */
	void AddTarget(ALrPawnBase* Target);

	/** 删除目标 */
	void RemoveTarget(ALrPawnBase* Target);

protected:
	/** 游戏开始时 */
	virtual void BeginPlay() override;

	/** 游戏结束时清理资源 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** 创建Beam */
	UNiagaraComponent* CreateBeam();

	/** 获取一个空闲Beam */
	FLrLightningBeam* AcquireBeam();

	/** 释放Beam */
	void ReleaseBeam(FLrLightningBeam& Beam);

	/** 更新所有Beam */
	void UpdateLightning();

	/** 更新一段Beam */
	void UpdateBeam(int32 BeamIndex);

	/** 获取Socket位置 */
	FVector GetSocketLocation(AActor* Actor, FName Socket) const;

	/** 删除无效目标 */
	void RemoveInvalidTargets();

	/** 目标死亡 */
	UFUNCTION()
	void OnTargetDestroyed(AActor* DestroyedActor);
	/** 每帧更新 Beam 位置与生命周期 */
	void Tick(float DeltaSeconds);

private:
	/** 施法者 */
	UPROPERTY()
	TObjectPtr<ALrPawnBase> Caster = nullptr;

	/** 链目标 */
	UPROPERTY()
	TArray<TWeakObjectPtr<ALrPawnBase>> Targets;
	
	// UPROPERTY()
	// TArray<TWeakObjectPtr<FLrTargetInfo>> Targets;

	
	/** Beam池 */
	UPROPERTY()
	TArray<FLrLightningBeam> BeamPool;

	/** Niagara模板 */
	UPROPERTY(EditDefaultsOnly, Category="Lightning")
	TObjectPtr<UNiagaraSystem> BeamSystem = nullptr;

	/** 最大Beam池子数量 */
	UPROPERTY(EditDefaultsOnly, Category="Lightning")
	int32 MaxBeamPoolCount = 16;

	/** 起点Socket */
	UPROPERTY(EditDefaultsOnly, Category="Lightning")
	FName StartSocket = TEXT("spine_03");

	/** 终点Socket */
	UPROPERTY(EditDefaultsOnly, Category="Lightning")
	FName EndSocket = TEXT("spine_03");

	/** 检查频率 */
	UPROPERTY(EditDefaultsOnly, Category="Lightning")
	float CheckFrequency = 0.2f;

	/** 攻击频率 */
	UPROPERTY(EditDefaultsOnly, Category="Lightning")
	float AttackFrequency = 0.2f;

	/** 最大连锁目标数 */
	UPROPERTY(EditDefaultsOnly, Category = "Lightning")
	int32 MaxNumShockTargets = 5;

	UPROPERTY()
	bool StopAttack = true;

	/** 已存在时间 */
	float ElapsedTime1 = 0.f;
	float ElapsedTime2 = 0.f;
	/** 是否需要刷新 */
	bool bDirty = true;
};
