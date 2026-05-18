// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrPawnBase.h"

#include "Component/LrAnimationComponent.h"
#include "Component/LrASComponent.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "Lib/LrCommonLibrary.h"
#include "Mover/FLrMoverInputCmd.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Nav/LrNavMovementComponent.h"

// Sets default values
ALrPawnBase::ALrPawnBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	// PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 关键：禁用Actor级别的移动复制
	// Mover组件有自己独立的网络预测和复制系统，不需要标准的Actor移动复制
	SetReplicatingMovement(false); // disable Actor-level movement replication, since our Mover component will handle it

	// =========================
	// Mover
	// =========================
	// LrMoverComponent = CreateDefaultSubobject<ULrMoverComponent>(TEXT("MoverComponent"));
	// todo 交给子类做
	// LrMoverComponent->SetUpdatedComponent(CapsuleComponent);
	CharacterMotionComponent = CreateDefaultSubobject<ULrMoverComponent>(TEXT("MoverComponent"));
	UE_LOG(LogTemp, Warning, TEXT("[ALrPawnBase init] on Mover=%p"),CharacterMotionComponent.Get());
	
	// =========================
	// Nav → Mover 桥接
	// =========================
	// LrNavMoverComponent = CreateDefaultSubobject<ULrNavMovementComponent>(TEXT("NavMoverComponent"));
	//  todo 交给子类做
	// LrNavMoverComponent->UpdatedComponent = CapsuleComponent;

	// =========================
	// AI 设置
	// =========================
	// AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	// AIControllerClass = AAIController::StaticClass();
	// bUseControllerRotationYaw = false;

	// =========================
	// 属性 ui设置
	// =========================
	LrASComponent = CreateDefaultSubobject<ULrASComponent>(TEXT("LrASComponent"));

	// =========================
	// 动画相关
	// =========================
	LrAnimationComponent = CreateDefaultSubobject<ULrAnimationComponent>(TEXT("LrAnimationComponent"));
}


UAbilitySystemComponent* ALrPawnBase::GetAbilitySystemComponent() const
{
	// 使得 UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor)可以拿到ASC
	return LrASC;
}

// Called when the game starts or when spawned
void ALrPawnBase::BeginPlay()
{
	Super::BeginPlay();
	FMoverInputCmdContext MoverInputCmdContext = CharacterMotionComponent->GetLastInputCmd();
	if (FMoverDataStructBase* MoverDataStructBase = MoverInputCmdContext.InputCollection.FindDataByType(FCharacterDefaultInputs::StaticStruct()))
	{
		FCharacterDefaultInputs::StaticStruct()->CopyScriptStruct(
			&CharacterDefaultInputsPre,
			MoverDataStructBase
		);
	}
}

void ALrPawnBase::ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult)
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}
	// IMoverInputProducerInterface::ProduceInput_Implementation(SimTimeMs, InputCmdResult);
	OnProduceInput(SimTimeMs, InputCmdResult);
}

// void ALrPawnBase::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
// {
// 	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
// 	FMoverInputCmdContext MoverInputCmdContext = CharacterMotionComponent->GetLastInputCmd();
// 	if (FMoverDataStructBase* MoverDataStructBase = MoverInputCmdContext.InputCollection.FindDataByType(FCharacterDefaultInputs::StaticStruct()))
// 	{
// 		FCharacterDefaultInputs::StaticStruct()->CopyScriptStruct(
// 			&CharacterDefaultInputsPre,
// 			MoverDataStructBase
// 		);
// 	}
// }


// void ALrPawnBase::OnProduceInput(float DeltaMs, FMoverInputCmdContext& InputCmdResult)
// {
// 	// CharacterMotionComponent.
// 	// 1️⃣ 清空“蓝图里的 InputDataCollection”
// 	// InputDataCollection.Empty();
// 	// CharacterDefaultInputsPre.SetMoveInput(EMoveInputType::DirectionalIntent, CachedMoveInput);
// 	// CharacterDefaultInputsPre.OrientationIntent = CachedMoveInput;
// 	// InputDataCollection.AddDataByCopy(&CharacterDefaultInputsPre);
// 	// InputCmdResult.InputCollection.AddDataByCopy(&CharacterDefaultInputsPre);
// 	// InputCmdResult.InputCollection = InputDataCollection;
//
// 	// FMoverDataCollection,K2_AddDataToCollection
// 	// CharacterDefaultInputsPre
// 	/**
// 	  * 关键步骤1：获取输入数据结构
// 	  * FCharacterDefaultInputs是派生自FMoverDataStructBase的结构
// 	  * 使用FindOrAddMutableDataByType获取或创建，存储在InputCollection中
// 	  * 生命周期：仅属于这一帧的InputCmd，模拟帧结束后丢弃
// 	  */
// 	// FLrDefaultInputs& Inputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FLrDefaultInputs>();
// 	UWorld* World = this->GetWorld();
//
// 	// ************************************************************************************************
// 	FCharacterDefaultInputs& Inputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();
// 	const FVector RawMove = CachedMoveInput;
// 	FVector MoveIntentLocal(RawMove.Y, RawMove.X, 0.f);
// 	
// 	if (!MoveIntentLocal.IsNearlyZero())
// 	{
// 		if (World->IsNetMode(NM_ListenServer))
// 		{
// 			UE_LOG(LogTemp, Warning, TEXT("Server===%s"), *CachedMoveInput.ToString());
// 		}
// 		else if (World->IsNetMode(NM_Client))
// 		{
// 			UE_LOG(LogTemp, Warning, TEXT("Client==%s"), *CachedMoveInput.ToString());
// 		}
//
// 		
// 		Inputs.SetMoveInput(EMoveInputType::DirectionalIntent, MoveIntentLocal.GetClampedToMaxSize(1.f));
// 		Inputs.OrientationIntent = MoveIntentLocal.GetSafeNormal();
// 		Inputs.SuggestedMovementMode = DefaultModeNames::Walking;
// 		if (World->IsNetMode(NM_ListenServer))
// 		{
// 			UE_LOG(LogTemp, Warning, TEXT("Server*****%s"), *Inputs.OrientationIntent.ToString());
// 		}
// 		else if (World->IsNetMode(NM_Client))
// 		{
// 			UE_LOG(LogTemp, Warning, TEXT("Client*********%s"), *Inputs.OrientationIntent.ToString());
// 		}
// 	}
// 	else
// 	{
// 		// ⭐ 正确的“停下”
// 		Inputs.SetMoveInput(EMoveInputType::DirectionalIntent, FVector::ZeroVector);
// 		// Inputs.OrientationIntent = FVector::ZeroVector;
// 		// Inputs.SuggestedMovementMode = NAME_None;
// 	}
// }

void ALrPawnBase::OnProduceInput(float DeltaMs, FMoverInputCmdContext& InputCmdResult)
{
	UWorld* World = this->GetWorld();

	FLrMoverInputCmd& Inputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FLrMoverInputCmd>();

	// 可选：检测地面物理材质，影响摩擦力（例如冰面）
	float FrictionMult = 1.0f;
	CheckFloorPhysics(FrictionMult);

	// 根据摩擦力调整输入强度
	FVector EffectiveInput = CachedMoveInput;
	if (FrictionMult < 0.5f)
	{
		EffectiveInput *= 0.2f; // Скользкий лед
	}

	// 填充输入数据
	Inputs.ControlRotation = GetControlRotation(); // 当前控制器旋转（相机朝向）

	if (!EffectiveInput.IsNearlyZero())
	{
		// 设置移动意图类型为方向性意图，并传入移动方向
		Inputs.SetMoveInput(EMoveInputType::DirectionalIntent, EffectiveInput);
		// 朝移动方向看（角色面向行走方向）
		Inputs.OrientationIntent = EffectiveInput; // Смотрим туда, куда идем
	}
	else
	{
		// 无移动输入：移动意图为零向量
		Inputs.SetMoveInput(EMoveInputType::DirectionalIntent, FVector::ZeroVector);
		// 保持当前角色朝向（或使用控制器前向，取决于设计）
		Inputs.OrientationIntent = GetActorForwardVector();
	}

	// 跳跃输入（一次性）
	Inputs.bIsJumpPressed = bIsJumpJustPressed;
	bIsJumpJustPressed = false; //
}


// ue的mover插件，客户端移动出现抖动，服务器的移动的画面没有问题
// Called every frame
void ALrPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ALrPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


/** 输入更新相关 */
void ALrPawnBase::UpdateMove(FVector Input)
{
	CachedMoveInput = Input;
}

void ALrPawnBase::UpdatePressedJump(bool Input)
{
	bIsJumpJustPressed = true;
}

/** 输入更新相关 */


void ALrPawnBase::CheckFloorPhysics(float& OutFrictionMult)
{
	OutFrictionMult = 1.0f;
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 100.0f);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bReturnPhysicalMaterial = true;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (Hit.PhysMaterial.IsValid())
		{
			OutFrictionMult = Hit.PhysMaterial->Friction;
		}
	}
}
