// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Corpse/LrCorpseActor.h"

#include "Data/LrCorpseConfigDA.h"

// Sets default values
ALrCorpseActor::ALrCorpseActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CorpseMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CorpseMesh"));
	RootComponent = CorpseMesh;
	// 配置物理预设
	CorpseMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	CorpseMesh->SetGenerateOverlapEvents(false);
}

USkeletalMeshComponent* ALrCorpseActor::InitializeCorpse(const USkeletalMeshComponent* SourceMesh, const FVector& Impulse, const FLrCorpseConfig& Config)
{
	if (!SourceMesh) return CorpseMesh;
	LifeDuration = Config.LifeDuration;

	//--------------------------------------------------
	// 1. 复制Mesh
	//--------------------------------------------------
	CorpseMesh->SetSkeletalMesh(SourceMesh->GetSkeletalMeshAsset());

	//--------------------------------------------------
	// 2. 复制材质
	//--------------------------------------------------
	for (int32 i = 0; i < SourceMesh->GetNumMaterials(); ++i)
	{
		CorpseMesh->SetMaterial(i, SourceMesh->GetMaterial(i));
	}

	//--------------------------------------------------
	// 3. 复制Transform
	//--------------------------------------------------
	SetActorTransform(SourceMesh->GetComponentTransform());

	//--------------------------------------------------
	// 4. PhysicsAsset
	//--------------------------------------------------
	if (Config.CustomPhysicsAsset)
	{
		CorpseMesh->SetPhysicsAsset(Config.CustomPhysicsAsset, false);
	}

	//--------------------------------------------------
	// 5. 复制当前姿态
	//--------------------------------------------------
	CorpseMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	CorpseMesh->SetSkeletalMesh(SourceMesh->GetSkeletalMeshAsset());
	SetActorTransform(SourceMesh->GetComponentTransform());

	// CorpseMesh->CopyPoseFromSkeletalComponent(
	// 	const_cast<USkeletalMeshComponent*>(SourceMesh));

	//--------------------------------------------------
	// 6. 刷新骨骼
	//--------------------------------------------------

	CorpseMesh->RefreshBoneTransforms();

	//--------------------------------------------------
	// 7. 开启布娃娃
	//--------------------------------------------------

	if (!CorpseMesh->GetPhysicsAsset())
	{
		UE_LOG(LogTemp, Error, TEXT("Corpse PhysicsAsset is NULL"));
		return CorpseMesh;
	}

	// UE_LOG(LogTemp, Warning, TEXT("PhysicsAsset=%s"), *GetNameSafe(CorpseMesh->GetPhysicsAsset()));

	CorpseMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	CorpseMesh->SetAllBodiesSimulatePhysics(true);

	CorpseMesh->WakeAllRigidBodies();

	CorpseMesh->bBlendPhysics = true;

	//--------------------------------------------------
	// 8. 检查是否成功
	//--------------------------------------------------

	UE_LOG(LogTemp, Warning, TEXT("IsSimulating=%d"), CorpseMesh->IsSimulatingPhysics());

	//--------------------------------------------------
	// 9. 添加冲力
	//--------------------------------------------------
	if (!Impulse.IsNearlyZero())
	{
		static const FName PelvisBone(TEXT("pelvis"));

		if (CorpseMesh->GetBoneIndex(PelvisBone) != INDEX_NONE)
		{
			CorpseMesh->AddImpulseToAllBodiesBelow(
				Impulse,
				PelvisBone,
				true,
				true);
		}
		else
		{
			CorpseMesh->AddImpulse(Impulse, NAME_None, true);
		}
	}

	//--------------------------------------------------
	// 10. 定时销毁
	//--------------------------------------------------
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		this,
		&ALrCorpseActor::OnCorpseLifeEnd,
		LifeDuration,
		false);

	return CorpseMesh;
}

void ALrCorpseActor::OnCorpseLifeEnd()
{
	// 这里也可以写渐隐（Dissolve）材质逻辑，写完后 Destroy
	Destroy();
}
