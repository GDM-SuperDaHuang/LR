// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/LrPawnBase.h"

#include "ASC/LrASC.h"
#include "ASC/AS/LrAS.h"
#include "Component/LrAnimationComponent.h"
#include "Data/LrExcelConfig.h"
#include "Lib/LrCommonLibrary.h"
#include "Mover/FLrMoverInputCmd.h"
#include "Mover/LrAllModes.h"
#include "Mover/LrMoverComponent.h"
#include "Mover/Air/LrAirMovementMode.h"
#include "Mover/Blink/LrBlinkMovementMode.h"
#include "Mover/Blink/LrKnockbackMovementMode.h"
#include "Mover/Death/LrDeathMovementMode.h"
#include "Mover/Walk/LrWalkMovementMode.h"

// Sets default values
ALrPawnBase::ALrPawnBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 关键：禁用Actor级别的移动复制
	// Mover组件有自己独立的网络预测和复制系统，不需要标准的Actor移动复制
	SetReplicatingMovement(false); // disable Actor-level movement replication, since our Mover component will handle it

	// =========================
	// Mover
	// =========================
	LrMoverComponent = CreateDefaultSubobject<ULrMoverComponent>(TEXT("MoverComponent"));
	UE_LOG(LogTemp, Warning, TEXT("[ALrPawnBase init] on Mover=%p"), LrMoverComponent.Get());

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

ULrCombatComponentBase* ALrPawnBase::GetCombatComponent() const
{
	return LrCombatComponent;
}


uint8 ALrPawnBase::GetTeamID() const
{
	return TeamID;
}

bool ALrPawnBase::IsDead() const
{
	return false;
}

ULrASC* ALrPawnBase::GetASC() const
{
	return LrASC;
}

ULrAS* ALrPawnBase::GetAS() const
{
	return LrAS;
}

uint8 ALrPawnBase::GetClassID() const
{
	return 0;
}

void ALrPawnBase::ToDie(const FVector& DeathImpulse, float Duration) const
{
	LrMoverComponent->Launch(DeathImpulse, Duration);

	// 1. 禁用碰撞和移动，防止死尸挡路或继续移动
	// LrCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// LrSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// GetCharacterMovement()->DisableMovement();
	//
	// // 2. 创建动态材质实例 (假设Mesh上使用的是支持溶解的材质)
	// UMaterialInterface* BaseMaterial = GetMesh()->GetMaterial(0);
	// if (BaseMaterial)
	// {
	// 	DynamicDissolveMaterial = GetMesh()->CreateDynamicMaterialInstance(0, BaseMaterial);
	// }
	//
	// // 3. 开启溶解状态
	// bIsDissolving = true;
	// DissolveValue = 0.0f;
}

void ALrPawnBase::HandleMoverFinalized(const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState)
{
	// 这里拿到的就是最精确的当前帧速度！
	FVector NewVelocity = LrMoverComponent->GetVelocity();
	float NewSpeed = NewVelocity.Size();
	float LastSize = LastVelocity.Size();
	// 只标记速度0.5作为临界值瞬间时，才有委托,注意混合空间
	// UE_LOG(LogTemp, Warning, TEXT("HandleMoverFinalized 新速度 = %f"), NewSpeed);
	// UE_LOG(LogTemp, Warning, TEXT("HandleMoverFinalized 老速度 = %f"), LrAnimationComponent->MovementData.Speed);
	if (NewSpeed > 0.5 && LastSize < 0.5) //0~0.5的时候
	{
		LastVelocity = NewVelocity;
		LrAnimationComponent->MovementData.Speed = NewSpeed;
		LrAnimationComponent->OnMovementDataChanged.ExecuteIfBound(LrAnimationComponent->MovementData);
		// UE_LOG(LogTemp, Warning, TEXT("HandleMoverFinalized 开始起步 = %f"), NewSpeed);
	}
	else if (NewSpeed < 0.5f && LastSize > 0.5) //回到0~0.5的时候
	{
		LastVelocity = NewVelocity;
		LrAnimationComponent->MovementData.Speed = NewSpeed;
		LrAnimationComponent->OnMovementDataChanged.ExecuteIfBound(LrAnimationComponent->MovementData);
		// UE_LOG(LogTemp, Warning, TEXT("HandleMoverFinalized 回退到起步 = %f"), NewSpeed);
	}
	if (LrMoverComponent->bJumpInitiated != LrAnimationComponent->MovementData.bIsJumping)
	{
		LrAnimationComponent->MovementData.bIsJumping = LrMoverComponent->bJumpInitiated;
		LrAnimationComponent->OnMovementDataChanged.ExecuteIfBound(LrAnimationComponent->MovementData);
		// UE_LOG(LogTemp, Warning, TEXT("HandleMoverFinalized bJumpInitiated "));
	}
}

void ALrPawnBase::HandleOnMovementModeChanged(const FName& PreviousMovementModeName, const FName& NewMovementModeName)
{
	if (NewMovementModeName == LrAllModes::Blink) //在闪现
	{
		LrAnimationComponent->MovementData.bIsBlink = true;
		LrAnimationComponent->OnMovementDataChanged.ExecuteIfBound(LrAnimationComponent->MovementData);
		// UE_LOG(LogTemp, Warning, TEXT("HandleOnMovementModeChanged Blink "));
	}
	else if (NewMovementModeName == LrAllModes::Air) //在空中
	{
		LrAnimationComponent->MovementData.bIsBlink = false;
		LrAnimationComponent->MovementData.bIsFalling = true;
		LrAnimationComponent->OnMovementDataChanged.ExecuteIfBound(LrAnimationComponent->MovementData);
		// UE_LOG(LogTemp, Warning, TEXT("HandleOnMovementModeChanged Air "));
	}
	else if (NewMovementModeName == LrAllModes::Walk) //刚刚落地
	{
		LrAnimationComponent->MovementData.bIsBlink = false;
		LrAnimationComponent->MovementData.bIsFalling = false;
		LrAnimationComponent->OnMovementDataChanged.ExecuteIfBound(LrAnimationComponent->MovementData);
		// UE_LOG(LogTemp, Warning, TEXT("HandleOnMovementModeChanged Walk "));
	}
}

// Called when the game starts or when spawned
void ALrPawnBase::BeginPlay()
{
	Super::BeginPlay();
	FMoverInputCmdContext MoverInputCmdContext = LrMoverComponent->GetLastInputCmd();
	if (FMoverDataStructBase* MoverDataStructBase = MoverInputCmdContext.InputCollection.FindDataByType(FCharacterDefaultInputs::StaticStruct()))
	{
		FCharacterDefaultInputs::StaticStruct()->CopyScriptStruct(
			&CharacterDefaultInputsPre,
			MoverDataStructBase
		);
	}
	// todo 销毁时候最好 RemoveDynamic
	LrMoverComponent->OnPostFinalize.AddDynamic(this, &ALrPawnBase::HandleMoverFinalized);
	LrMoverComponent->OnMovementModeChanged.AddDynamic(this, &ALrPawnBase::HandleOnMovementModeChanged);


	//  清空旧模式（防止重复注册）
	LrMoverComponent->MovementModes.Empty();

	// 创建新的移动模式对象：行走模式、空中模式
	// 使用 RealisticMovementDefines 中定义的键名（例如 RealisticModes::Walk
	LrMoverComponent->MovementModes.Add(LrAllModes::Walk, NewObject<ULrWalkMovementMode>(LrMoverComponent));
	LrMoverComponent->MovementModes.Add(LrAllModes::Air, NewObject<ULrAirMovementMode>(LrMoverComponent));
	LrMoverComponent->MovementModes.Add(LrAllModes::Blink, NewObject<ULrBlinkMovementMode>(LrMoverComponent));
	LrMoverComponent->MovementModes.Add(LrAllModes::Knock, NewObject<ULrKnockbackMovementMode>(LrMoverComponent));
	LrMoverComponent->MovementModes.Add(LrAllModes::Death, NewObject<ULrDeathMovementMode>(LrMoverComponent));

	// 清空显式的状态转换表（转换逻辑已内置于各移动模式内部）
	LrMoverComponent->Transitions.Empty();

	// 设置起始模式为空中模式（防止角色一开始就卡在地面下）
	// 通常空中模式会在落地时自动切换到行走模式
	// Устанавливаем стартовый режим
	LrMoverComponent->QueueNextMode(LrAllModes::Air);
}

void ALrPawnBase::ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult)
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}
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
		// UE_LOG(LogTemp, Warning, TEXT("OnProduceInput : %f"), CachedMoveInput.Length());
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

FGenericTeamId ALrPawnBase::GetGenericTeamId() const
{
	return FGenericTeamId(TeamID);
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


void ALrPawnBase::InitAS() const
{
	ALrGameModeBase* LrGameModeBase = ULrCommonLibrary::GetLrGameModeBase(GetWorld());
	TObjectPtr<ULrExcelConfig> LrExcelConfig = LrGameModeBase->LrExcelConfig;
	if (!LrExcelConfig) return;
	TArray<FLrInitASRow*> LrInitAsRows = LrExcelConfig->FindAllAS();
	for (const FLrInitASRow* Row : LrInitAsRows)
	{
		if (!Row)
		{
			continue;
		}
		if (!Row->ASTag.IsValid())
		{
			continue;
		}
		FAttributeFuncPtr* Find = LrAS->TagsASMap.Find(Row->ASTag);
		if (Find)
		{
			FGameplayAttribute GameplayAttribute = (*Find)();
			LrASC->SetNumericAttributeBase(GameplayAttribute, Row->BaseValue);
		}
	}
}
