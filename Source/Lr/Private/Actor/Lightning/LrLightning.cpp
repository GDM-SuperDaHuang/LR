// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Lightning/LrLightning.h"

#include "Components/SkeletalMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "ASC/LrASC.h"
#include "Component/LrAnimationComponent.h"
#include "Mover/LrMoverComponent.h"
#include "Pawn/LrPawnBase.h"


// 启用每帧 Tick，用于跟随移动目标与计时销毁
ALrLightning::ALrLightning()
{
	PrimaryActorTick.bCanEverTick = true;
}

//攻击
void ALrLightning::Attack(ALrPawnBase* InCaster)
{
	if (Targets.Num() > 0)
	{
		FVector TargetLocation = Targets[0]->GetActorLocation();
		FVector OwnerLocation = InCaster->GetActorLocation();
		// 计算从攻击者指向目标的方向旋转
		FRotator TargetRotation = (TargetLocation - OwnerLocation).Rotation();
		// 通常攻击转向只需要水平转身（Yaw），需要把 Pitch 和 Roll 清零，防止角色发生倾斜
		TargetRotation.Pitch = 0.f;
		TargetRotation.Roll = 0.f;
		InCaster->LrMoverComponent->AttackWarpRotation = TargetRotation;
	}

	for (TWeakObjectPtr<ALrPawnBase> Target : Targets)
	{
		if (!InCaster || Target == InCaster || !Target.Get())
		{
			continue;
		}
		if (ULrASC* LrASC = Target->GetASC())
		{
			FDamageEffectParams DamageEffectParams;
			DamageEffectParams.AddFlag(EDamageFlags::Stiffness);
			DamageEffectParams.DamageValue = 5;
			LrASC->ApplyDamageToTarget(Caster, Target.Get(), DamageEffectParams);
			InCaster->LrMoverComponent->bIsInAttackWarp = true;
		}

		//发布
		Target->LrAnimationComponent->MovementData.Status = 1;
		Target->LrAnimationComponent->OnMovementDataChanged.ExecuteIfBound(Target->LrAnimationComponent->MovementData);
	}
}

// 初始化链式闪电：记录施法者，清空旧数据并注册所有目标
void ALrLightning::Init(ALrPawnBase* InCaster)
{
	StopAttack = false;
	Caster = InCaster;
	Clear();
	ULrCombatComponentBase* Combat = InCaster->GetCombatComponent();
	if (Combat == nullptr)
	{
		return;
	}
	FLrCombatQueryParams ConeParams;
	ConeParams.Origin = InCaster->GetActorLocation();
	ConeParams.Forward = InCaster->GetActorForwardVector();
	ConeParams.ShapeType = ELrCombatShape::Sphere;
	ConeParams.Radius = 1200.f;
	TArray<TWeakObjectPtr<AActor>> EnemiesInCone = Combat->PerformConeQuery(ConeParams);
	int32 Max = 0;
	Combat->SortActorsByDistance(EnemiesInCone, Caster->GetActorLocation());
	// 收集有效目标（限制连锁数量）
	for (TWeakObjectPtr<AActor>& Enemy : EnemiesInCone)
	{
		if (ALrPawnBase* TargetPawn = Cast<ALrPawnBase>(Enemy.Get()))
		{
			AddTarget(TargetPawn);
			Max++;
		}
		if (MaxNumShockTargets <= Max)
		{
			break;
		}
	}
	bDirty = true;
}

void ALrLightning::Stop()
{
	Caster->LrMoverComponent->bIsInAttackWarp = false;
	StopAttack = true;
	Clear();
}

// 释放所有 Beam 并解绑目标死亡监听
void ALrLightning::Clear()
{
	for (FLrLightningBeam& Beam : BeamPool)
	{
		ReleaseBeam(Beam);
	}
	for (TWeakObjectPtr<ALrPawnBase> Target : Targets)
	{
		if (Target.Get())
		{
			Target->OnDestroyed.RemoveDynamic(this, &ALrLightning::OnTargetDestroyed);
		}
		//发布
		Target->LrAnimationComponent->MovementData.Status = 0;
		Target->LrAnimationComponent->OnMovementDataChanged.ExecuteIfBound(Target->LrAnimationComponent->MovementData);
	}
	Targets.Empty();
	bDirty = true;
}

// LrAnimationComponent->MovementData.bIsJumping = LrMoverComponent->bJumpInitiated;

// 添加目标并绑定死亡监听，重复目标会被忽略
void ALrLightning::AddTarget(ALrPawnBase* Target)
{
	if (!Target || Targets.Contains(Target)) return;
	Targets.Add(Target);
	//添加监听
	Target->OnDestroyed.AddDynamic(this, &ALrLightning::OnTargetDestroyed);
	bDirty = true;
}

// 移除目标并解绑死亡监听
void ALrLightning::RemoveTarget(ALrPawnBase* Target)
{
	Targets.Remove(Target);
	bDirty = true;
}

// Actor 销毁前清理所有资源与监听
void ALrLightning::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Clear();
	Super::EndPlay(EndPlayReason);
}

// 基于 BeamSystem 模板创建一个新的 Niagara Beam 组件，交给对象池管理
UNiagaraComponent* ALrLightning::CreateBeam()
{
	if (!BeamSystem)
	{
		return nullptr;
	}
	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
		BeamSystem,
		GetRootComponent(),
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepWorldPosition,
		false);
	if (NiagaraComp)
	{
		NiagaraComp->bAutoActivate = false;
		NiagaraComp->SetAutoDestroy(false);
	}
	return NiagaraComp;
}

// 从对象池中获取一个空闲 Beam；没有空闲且未达上限时创建新的
FLrLightningBeam* ALrLightning::AcquireBeam()
{
	for (FLrLightningBeam& Beam : BeamPool)
	{
		if (!Beam.bActive) return &Beam;
	}

	// 局部变量不能返回
	FLrLightningBeam NewBeam;
	NewBeam.NiagaraComponent = CreateBeam();
	if (!NewBeam.NiagaraComponent)
	{
		return nullptr;
	}
	int32 NewIndex = 0;
	if (BeamPool.Num() < MaxBeamPoolCount) // 加入池
	{
		NewIndex = BeamPool.Add(NewBeam);
	}
	else
	{
		return nullptr;
	}
	// 返回池中新元素的地址（⚠️ 注意：后续Add可能引发TArray重分配，导致此指针失效）
	return &BeamPool[NewIndex];
}

// 重置 Beam 引用并停用 Niagara 组件，使其回到对象池
void ALrLightning::ReleaseBeam(FLrLightningBeam& Beam)
{
	Beam.bActive = false;
	Beam.StartActor = nullptr;
	Beam.EndActor = nullptr;
	if (Beam.NiagaraComponent)
	{
		Beam.NiagaraComponent->Deactivate();
	}
}

// 重新分配 Beam 到当前目标链：Caster->T0->T1->...Tn->Tn+1
void ALrLightning::UpdateLightning()
{
	UE_LOG(LogTemp, Warning, TEXT("======UpdateLightning"));
	// 去除所有的 Beam 重新分配
	for (FLrLightningBeam& Beam : BeamPool)
	{
		if (Beam.bActive)
		{
			ReleaseBeam(Beam);
		}
	}

	// 可能需要补充新敌人
	const int32 NeededBeams = FMath::Min(Targets.Num(), MaxBeamPoolCount);
	for (int32 i = 0; i < NeededBeams; ++i)
	{
		FLrLightningBeam* Beam = AcquireBeam();
		if (!Beam) break;
		Beam->bActive = true;
		Beam->StartActor = (i == 0) ? Caster : Targets[i - 1];
		Beam->EndActor = Targets[i];
		if (Beam->NiagaraComponent)
		{
			// Beam->NiagaraComponent->AttachToComponent(Beam->StartActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			UE_LOG(LogTemp, Warning, TEXT("======UpdateLightning Activate"));
			Beam->NiagaraComponent->Activate(true);
		}
	}

	for (int32 i = 0; i < BeamPool.Num(); ++i)
	{
		if (BeamPool[i].bActive)
		{
			UpdateBeam(i);
		}
	}
}

// 根据 Beam 索引更新 Niagara 的 BeamStart 与 BeamEnd 参数
void ALrLightning::UpdateBeam(int32 BeamIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("======UpdateBeam"));

	if (!BeamPool.IsValidIndex(BeamIndex)) return;
	FLrLightningBeam& Beam = BeamPool[BeamIndex];
	if (!Beam.bActive || !Beam.NiagaraComponent) return;
	const FVector StartLocation = Beam.StartActor.Get() ? GetSocketLocation(Beam.StartActor.Get(), StartSocket) : GetActorLocation();
	const FVector EndLocation = Beam.EndActor.Get() ? GetSocketLocation(Beam.EndActor.Get(), EndSocket) : GetActorLocation();
	// Beam.NiagaraComponent->SetVariablePosition(TEXT("UserBeamStart"), StartLocation);//User.BeamStart
	// Beam.NiagaraComponent->SetVariablePosition(TEXT("UserBeamEnd"), EndLocation);
	Beam.NiagaraComponent->SetVectorParameter(TEXT("Start0"), StartLocation);
	Beam.NiagaraComponent->SetVectorParameter(TEXT("End0"), EndLocation);

	Beam.NiagaraComponent->SetVisibility(true);
	UE_LOG(LogTemp, Warning, TEXT("Beam %s -> %s Dist=%f"),
	       *StartLocation.ToString(),
	       *EndLocation.ToString(),
	       FVector::Dist(StartLocation,EndLocation));
	UE_LOG(LogTemp, Warning, TEXT("======SetVectorParameter"));
	// 	(X=1180.000000,Y=710.000000,Z=79.600100)
	// (X=1110.000000,Y=1080.000000,Z=79.600100)
}

// 获取 Actor 上指定 Socket 的位置；Socket 不存在或无 Socket 时回退到 Actor 位置
FVector ALrLightning::GetSocketLocation(AActor* Actor, FName Socket) const
{
	if (!Actor) return FVector::ZeroVector;
	if (Socket == NAME_None) return Actor->GetActorLocation();
	if (USceneComponent* RootComp = Actor->GetRootComponent())
	{
		if (RootComp->DoesSocketExist(Socket))
		{
			return RootComp->GetSocketLocation(Socket);
		}
	}
	if (USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (SkeletalMeshComp->DoesSocketExist(Socket))
		{
			return SkeletalMeshComp->GetSocketLocation(Socket);
		}
	}
	return Actor->GetActorLocation();
}

// 清理已销毁或无效目标，并触发 Beam 重新分配
void ALrLightning::RemoveInvalidTargets()
{
	for (int32 i = Targets.Num() - 1; i >= 0; --i)
	{
		TWeakObjectPtr<ALrPawnBase> Target = Targets[i];
		if (!Target.IsValid()) // Target调用Destroy时
		{
			Targets.RemoveAt(i);
			bDirty = true;
		}
	}
}

// 目标死亡回调：从目标列表中移除
void ALrLightning::OnTargetDestroyed(AActor* DestroyedActor)
{
	if (ALrPawnBase* Target = Cast<ALrPawnBase>(DestroyedActor))
	{
		RemoveTarget(Target);
	}
}

// 每帧推进生命周期，目标变化时重分配 Beam，否则仅更新起止位置
void ALrLightning::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (StopAttack)
	{
		return;
	}
	ElapsedTime1 += DeltaSeconds;
	ElapsedTime2 += DeltaSeconds;
	if (ElapsedTime1 >= AttackFrequency)
	{
		ElapsedTime1 = 0;
		// 攻击
		Attack(Caster);
	}
	if (ElapsedTime2 >= CheckFrequency)
	{
		ElapsedTime2 = 0;
		return;
	}

	RemoveInvalidTargets();
	if (bDirty)
	{
		UpdateLightning();
		bDirty = false;
	}
	else
	{
		for (int32 i = 0; i < BeamPool.Num(); ++i)
		{
			if (BeamPool[i].bActive)
			{
				UpdateBeam(i);
			}
		}
	}
}

// 游戏开始时调用父类逻辑
void ALrLightning::BeginPlay()
{
	Super::BeginPlay();
}
