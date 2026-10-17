// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LrDestructibleActor.generated.h"

class ULrDestructionComponent;

UCLASS()
class LR_API ALrDestructibleActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALrDestructibleActor();

	virtual void BeginPlay() override;
	//测试
	// void TestBreak() const;
	// UPROPERTY(EditAnywhere)
	// bool bTestBreak =false;

	// 子弹 / 投射物命中统一入口：唤醒 + 碎裂
	UFUNCTION(BlueprintCallable, Category = "LR|Destruction")
	bool NotifyBulletHit(const FHitResult& Hit, const FVector& BulletVelocity, float Damage = 100.f);

	/**
	 * 销毁碎裂碎片：停止物理模拟 + 关闭碰撞 + 隐藏渲染。
	 * 子弹命中产生碎片后 FragmentLifeTime 秒自动调用，也可手动 / 蓝图调用。
	 */
	UFUNCTION(BlueprintCallable, Category = "LR|Destruction")
	void DestroyFragments();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGeometryCollectionComponent> GeometryCollection;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULrDestructionComponent> Destruction;

	/** 碎片从产生到被销毁的存活时间（秒），<=0 表示不自动销毁 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LR|Destruction")
	float FragmentLifeTime = 6.0f;

private:
	/** 碎片自动销毁计时器 */
	FTimerHandle FragmentDestroyTimerHandle;
};
