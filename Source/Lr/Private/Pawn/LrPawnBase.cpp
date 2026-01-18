// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrPawnBase.h"

#include "Mover/LrMoverComponent.h"
#include "Mover/Nav/LrNavMovementComponent.h"

// Sets default values
ALrPawnBase::ALrPawnBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// bReplicates = true;
	
	// 关键：禁用Actor级别的移动复制
	// Mover组件有自己独立的网络预测和复制系统，不需要标准的Actor移动复制
	SetReplicatingMovement(false); // disable Actor-level movement replication, since our Mover component will handle it

	// =========================
	// Mover
	// =========================
	// LrMoverComponent = CreateDefaultSubobject<ULrMoverComponent>(TEXT("MoverComponent"));
	// todo 交给子类做
	// LrMoverComponent->SetUpdatedComponent(CapsuleComponent);

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
	
}

// Called when the game starts or when spawned
void ALrPawnBase::BeginPlay()
{
	Super::BeginPlay();
}

void ALrPawnBase::ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult)
{
	// IMoverInputProducerInterface::ProduceInput_Implementation(SimTimeMs, InputCmdResult);
	OnProduceInput(SimTimeMs, InputCmdResult);
}

void ALrPawnBase::OnProduceInput(float DeltaMs, FMoverInputCmdContext& InputCmdResult)
{
	
	/**
	  * 关键步骤1：获取输入数据结构
	  * FCharacterDefaultInputs是派生自FMoverDataStructBase的结构
	  * 使用FindOrAddMutableDataByType获取或创建，存储在InputCollection中
	  * 生命周期：仅属于这一帧的InputCmd，模拟帧结束后丢弃
	  */
	// FLrDefaultInputs& Inputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FLrDefaultInputs>();
	FCharacterDefaultInputs& Inputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();
	const FVector2D RawMove2D = CachedMoveInput;
	FVector MoveIntentLocal(RawMove2D.Y, RawMove2D.X, 0.f);

	if (!MoveIntentLocal.IsNearlyZero())
	{
		Inputs.SetMoveInput(EMoveInputType::DirectionalIntent,MoveIntentLocal.GetClampedToMaxSize(1.f));
		Inputs.OrientationIntent = MoveIntentLocal.GetSafeNormal();
		Inputs.SuggestedMovementMode = DefaultModeNames::Walking;
	}
	else
	{
		// ⭐ 正确的“停下”
		Inputs.SetMoveInput(EMoveInputType::DirectionalIntent,FVector::ZeroVector);
		// Inputs.OrientationIntent = FVector::ZeroVector;
		// Inputs.SuggestedMovementMode = NAME_None;
	}
	// todo 清零再别的地方
	// CachedMoveInput = FVector2D::ZeroVector;


	// // 如果没有控制器且是服务器上的模拟代理，提供默认空输入
	// // 这是为网络同步考虑：未控制的Pawn不应接收输入
	// if (GetController() == nullptr)
	// {
	// 	if (GetLocalRole() == ENetRole::ROLE_Authority && GetRemoteRole() == ENetRole::ROLE_SimulatedProxy)
	// 	{
	// 		// static const FLrDefaultInputs DoNothingInput;
	// 		static const FCharacterDefaultInputs DoNothingInput;
	// 		// If we get here, that means this pawn is not currently possessed and we're choosing to provide default do-nothing input
	// 		// 如果我们到这里，意味着这个pawn当前未被控制，我们提供默认的"什么都不做"输入
	// 		Inputs = DoNothingInput;
	// 	}
	//
	// 	// We don't have a local controller so we can't run the code below. This is ok. Simulated proxies will just use previous input when extrapolating
	// 	return;
	// }
	//
	// // ---------------------------------------------------------------------
	// // 2. 读取“原始移动输入”（通常来自 Enhanced Input）
	// //    这里假设你已经在 Pawn 里缓存了
	// // ---------------------------------------------------------------------
	// // 例如：CachedMoveInput = FVector2D(X, Y)
	// const FVector2D RawMove2D = CachedMoveInput; // [-1,1]
	//
	// // 转成 Forward / Right
	// FVector MoveIntentLocal(RawMove2D.Y, RawMove2D.X, 0.f);
	//
	// // ---------------------------------------------------------------------
	// // 3. 写入 MoveInput
	// // ---------------------------------------------------------------------
	// if (!MoveIntentLocal.IsNearlyZero())
	// {
	// 	// Inputs.SetMoveInput(ELrMoveInputType::DirectionalIntent,MoveIntentLocal.GetClampedToMaxSize(1.f));
	// 	Inputs.SetMoveInput(EMoveInputType::DirectionalIntent,MoveIntentLocal.GetClampedToMaxSize(1.f));
	//
	// }
	// else
	// {
	// 	Inputs.SetMoveInput(EMoveInputType::None, FVector::ZeroVector);
	// 	// Inputs.SetMoveInput(ELrMoveInputType::None, FVector::ZeroVector);
	// }
	//
	// // ---------------------------------------------------------------------
	// // 4. 控制旋转（极其重要）
	// // ---------------------------------------------------------------------
	// Inputs.ControlRotation = GetControlRotation();
	//
	// // ---------------------------------------------------------------------
	// // 5. 朝向意图（用于 Walk 朝向 / 动画）
	// // ---------------------------------------------------------------------
	// if (!MoveIntentLocal.IsNearlyZero())
	// {
	// 	Inputs.OrientationIntent = MoveIntentLocal.GetSafeNormal();
	// }
	// else
	// {
	// 	Inputs.OrientationIntent = FVector::ZeroVector;
	// }
	//
	// // ---------------------------------------------------------------------
	// // 6. 建议进入 Walk 模式（不是强制）
	// // ---------------------------------------------------------------------
	// Inputs.SuggestedMovementMode = TEXT("Walk");
	//
	// // ---------------------------------------------------------------------
	// // 7. 跳跃（留给 JumpMode 用）
	// // ---------------------------------------------------------------------
	// Inputs.bIsJumpJustPressed = bIsJumpJustPressed;
	// Inputs.bIsJumpPressed = bIsJumpPressed;
}

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
void ALrPawnBase::UpdateMove(FVector2D Input)
{
	CachedMoveInput = Input;
}

void ALrPawnBase::UpdatePressedJump(bool Input)
{
	bIsJumpJustPressed = true;
}

/** 输入更新相关 */
