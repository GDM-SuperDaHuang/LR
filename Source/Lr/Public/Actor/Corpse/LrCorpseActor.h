// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/LrCorpseConfigDA.h"
#include "GameFramework/Actor.h"
#include "LrCorpseActor.generated.h"

UCLASS()
class LR_API ALrCorpseActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALrCorpseActor();
	// 骨骼网格体
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* CorpseMesh;
	// 初始化尸体：传入原敌人的 Mesh 用于复制姿态，以及死亡冲量
	USkeletalMeshComponent* InitializeCorpse(const USkeletalMeshComponent* SourceMesh, const FVector& Impulse, const FLrCorpseConfig& Config);
protected:

	// 尸体存在的时间（秒），之后自动消失
	UPROPERTY(EditAnywhere, Category = "Corpse Settings")
	float LifeDuration = 5.0f;

	void OnCorpseLifeEnd();
	

};
