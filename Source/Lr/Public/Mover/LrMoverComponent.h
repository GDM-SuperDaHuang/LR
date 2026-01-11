// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MoverComponent.h"
#include "LrMoverComponent.generated.h"

UENUM(BlueprintType)
enum class ELrMoveInputType : uint8
{
	Invalid,

	/** Move with intent, as a per-axis magnitude [-1,1] (E.g., "move straight forward as fast as possible" would be (1, 0, 0) and "move straight left at half speed" would be (0, -0.5, 0) regardless of frame time). Zero vector indicates intent to stop.
	 * 玩家推满右摇杆 → (1,0,0)
	 * 玩家轻推 → (0.3,0,0)
	 * 摇杆回中 → (0,0,0)  立刻触发“减速到停”
	 * FVector(0.5f, -0.2f, 0) 半速向前、微微向右、不上下
	 *
	 * FLrDefaultInputs Inputs;
	 * Inputs.SetMoveInput(EMoveInputType::DirectionalIntent, FVector(0.5f, -0.2f, 0));
	 */
	DirectionalIntent,

	/** Move with a given velocity (units per second) */
	Velocity,

	/** No move input of any type */
	None,
};

/**
 * 
 */
UCLASS()
class LR_API ULrMoverComponent : public UMoverComponent
{
	GENERATED_BODY()

public:
	ULrMoverComponent();

	
	// virtual void InitializeComponent() override;
public:
	virtual void OnRegister() override;
};


// -----------------------------------------------------------------------------
// 二、FLrDefaultInputs —— 一帧内“玩家/AI 到底想干嘛”的完整自描述包
// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FLrDefaultInputs : public FMoverDataStructBase
{
	GENERATED_BODY()

	//-------- 下面 2 个函数是“规范入口” --------
	void SetMoveInput(ELrMoveInputType InMoveInputType, const FVector& InMoveInput);

	const FVector& GetMoveInput() const { return MoveInput; }
	ELrMoveInputType GetMoveInputType() const { return MoveInputType; }

protected:
	// 真正的存储变量，全部用 UPROPERTY 标记，方便反射、网络复制、蓝图只读
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mover)
	ELrMoveInputType MoveInputType;

	/**
	 * MoveInput：含义由 ELrMoveInputType 决定。
	 * - 如果是 DirectionalIntent，则为 [-1,1] 意图；
	 * - 如果是 Velocity，则为 cm/s。
	 * 若站在 MovementBase 上，则向量相对于该 base；否则是世界空间。
	 * 出于网络压缩，内部会被截断到定点精度。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mover)
	FVector MoveInput;

public:
	// 意图面向方向（归一化）。零向量表示“不想转”。
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mover)
	FVector OrientationIntent;

	// 控制旋转（通常是摄像机）的世界空间快照，用于“将输入从相机空间转到世界”
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mover)
	FRotator ControlRotation;

	// 强制建议进入某个移动模式（如“走路”“游泳”“攀爬”）。空值表示不干预。
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mover)
	FName SuggestedMovementMode;

	// 是否站在移动基座（电梯、飞船、传送带）上
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mover)
	bool bUsingMovementBase;

	// 可选：基座组件指针
	UPROPERTY(BlueprintReadWrite, Category = Mover)
	TObjectPtr<UPrimitiveComponent> MovementBase;

	// 可选：若基座是骨骼网格，这里记录绑定的骨骼名,如果基座是骨骼网格（如飞船骨架），可以精确到“踩在哪个骨骼”。
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mover)
	FName MovementBaseBoneName;

	// 跳按键状态
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mover)
	bool bIsJumpJustPressed; // 本帧刚刚按下
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mover)
	bool bIsJumpPressed; // 当前处于按住状态

	// -------- 工具函数：把“相对基座”的量转成世界空间 --------
	FVector GetMoveInput_WorldSpace() const;
	FVector GetOrientationIntentDir_WorldSpace() const;

	//-------- 构造 & 运算符 --------
	FLrDefaultInputs()
		: MoveInputType(ELrMoveInputType::None)
		  , MoveInput(ForceInitToZero)
		  , OrientationIntent(ForceInitToZero)
		  , ControlRotation(ForceInitToZero)
		  , SuggestedMovementMode(NAME_None)
		  , bUsingMovementBase(false)
		  , MovementBase(nullptr)
		  , MovementBaseBoneName(NAME_None)
		  , bIsJumpJustPressed(false)
		  , bIsJumpPressed(false)
	{
	}

	virtual ~FLrDefaultInputs()
	{
	}

	// 值相等比较（用于预测/回滚验证）
	bool operator==(const FLrDefaultInputs& Other) const
	{
		return MoveInputType == Other.MoveInputType
			&& MoveInput == Other.MoveInput
			&& OrientationIntent == Other.OrientationIntent
			&& ControlRotation == Other.ControlRotation
			&& SuggestedMovementMode == Other.SuggestedMovementMode
			&& bUsingMovementBase == Other.bUsingMovementBase
			&& MovementBase == Other.MovementBase
			&& MovementBaseBoneName == Other.MovementBaseBoneName
			&& bIsJumpJustPressed == Other.bIsJumpJustPressed
			&& bIsJumpPressed == Other.bIsJumpPressed;
	}

	bool operator!=(const FLrDefaultInputs& Other) const { return !operator==(Other); }

	//-------- FMoverDataStructBase 接口 --------
	virtual FMoverDataStructBase* Clone() const override;
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
	virtual void ToString(FAnsiStringBuilderBase& Out) const override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override { Super::AddReferencedObjects(Collector); }
	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override;
	virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct) override;
	virtual void Merge(const FMoverDataStructBase& From) override;
	virtual void Decay(float DecayAmount) override;
};

// 显式告诉 UE 的模板系统：这个结构要支持网络序列化与拷贝
template <>
struct TStructOpsTypeTraits<FLrDefaultInputs> : public TStructOpsTypeTraitsBase2<FLrDefaultInputs>
{
	enum { WithNetSerializer = true, WithCopy = true };
};

// -----------------------------------------------------------------------------
// 三、FLrMoverDefaultSyncState —— 服务器/客户端都要同步的“最小公共状态”
// -----------------------------------------------------------------------------
// Data block containing basic sync state information
USTRUCT(BlueprintType)
struct FLrMoverDefaultSyncState : public FMoverDataStructBase
{
	GENERATED_BODY()

protected:
	// 下面 4 个核心量默认“相对基座”，如果没有基座就是世界空间
	// Position relative to MovementBase if set, world space otherwise
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	FVector Location;

	// Forward-facing rotation relative to MovementBase if set, world space otherwise.
	//如果已设置，则相对于“移动基座”进行向前旋转；否则在世界空间中进行旋转。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	FRotator Orientation;

	// Linear velocity, units per second, relative to MovementBase if set, world space otherwise.
	// 线速度，单位为秒，若已设置则相对于“移动基座”计算，否则在世界空间中计算。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	FVector Velocity;

	// Angular velocity, degrees per second, relative to MovementBase if set, world space otherwise.
	// 角速度，每秒度数，若已设置则相对于“运动基座”计算，否则在世界空间中计算。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	FVector AngularVelocityDegrees;

public:
	// 如果已设置，则表示相对于“移动基础”的移动意图方向；否则为世界空间中的方向。范围的大小（0 - 1） ,意图方向（长度 0~1），用于动画/预测，相对基座或世界
	// Movement intent direction relative to MovementBase if set, world space otherwise. Magnitude of range (0-1)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	FVector MoveDirectionIntent;

protected:
	// 基座相关快照，用于“把相对量重建成世界量”
	// Optional: when moving on a base, input may be relative to this object
	// 可选：在移动至某个基点时，输入值可以相对于此物体进行设定。
	UPROPERTY(BlueprintReadOnly, Category = Mover)
	TWeakObjectPtr<UPrimitiveComponent> MovementBase;

	// Optional: for movement bases that are skeletal meshes, this is the bone we're based on. Only valid if MovementBase is set.
	UPROPERTY(BlueprintReadOnly, Category = Mover)
	FName MovementBaseBoneName;

	UPROPERTY(BlueprintReadOnly, Category = Mover)
	FVector MovementBasePos; // 捕获时的位置

	UPROPERTY(BlueprintReadOnly, Category = Mover)
	FQuat MovementBaseQuat; // 捕获时的旋转

public:
	FLrMoverDefaultSyncState()
		: Location(ForceInitToZero)
		  , Orientation(ForceInitToZero)
		  , Velocity(ForceInitToZero)
		  , AngularVelocityDegrees(ForceInitToZero)
		  , MoveDirectionIntent(ForceInitToZero)
		  , MovementBase(nullptr)
		  , MovementBaseBoneName(NAME_None)
		  , MovementBasePos(ForceInitToZero)
		  , MovementBaseQuat(FQuat::Identity)
	{
	}

	virtual ~FLrMoverDefaultSyncState()
	{
	}

	//-------- FMoverDataStructBase 接口 --------
	// @return newly allocated copy of this FLrMoverDefaultSyncState. Must be overridden by child classes
	virtual FMoverDataStructBase* Clone() const override;

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }

	virtual void ToString(FAnsiStringBuilderBase& Out) const override;

	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override;

	virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct) override;

	//-------- 核心工具：设置/更新基座 --------
	UE_DEPRECATED(5.7, "Use the SetTransforms_WorldSpace with an angular velocity")
	void SetTransforms_WorldSpace(FVector WorldLocation, FRotator WorldOrient, FVector WorldVelocity, UPrimitiveComponent* Base = nullptr, FName BaseBone = NAME_None);

	void SetTransforms_WorldSpace(FVector WorldLocation, FRotator WorldOrient, FVector WorldVelocity, FVector WorldAngularVelocityDegrees, UPrimitiveComponent* Base = nullptr, FName BaseBone = NAME_None);

	// Returns whether the base setting succeeded
	bool SetMovementBase(UPrimitiveComponent* Base, FName BaseBone = NAME_None);

	// Refreshes captured movement base transform based on its current state, while maintaining the same base-relative transforms
	bool UpdateCurrentMovementBase();

	// Queries
	bool IsNearlyEqual(const FLrMoverDefaultSyncState& Other) const;

	//-------- 查询：世界空间 & 基座空间 双版本 --------
	UPrimitiveComponent* GetMovementBase() const { return MovementBase.Get(); }
	FName GetMovementBaseBoneName() const { return MovementBaseBoneName; }
	FVector GetCapturedMovementBasePos() const { return MovementBasePos; }
	FQuat GetCapturedMovementBaseQuat() const { return MovementBaseQuat; }

	FVector GetLocation_WorldSpace() const;
	FVector GetLocation_BaseSpace() const; // If there is no movement base set, these will be the same as world space

	FVector GetIntent_WorldSpace() const;
	FVector GetIntent_BaseSpace() const;

	FVector GetVelocity_WorldSpace() const;
	FVector GetVelocity_BaseSpace() const;

	FRotator GetOrientation_WorldSpace() const;
	FRotator GetOrientation_BaseSpace() const;

	FTransform GetTransform_WorldSpace() const;
	FTransform GetTransform_BaseSpace() const;

	FVector GetAngularVelocityDegrees_WorldSpace() const;
	FVector GetAngularVelocityDegrees_BaseSpace() const;
};

template <>
struct TStructOpsTypeTraits<FLrMoverDefaultSyncState> : public TStructOpsTypeTraitsBase2<FLrMoverDefaultSyncState>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

// -----------------------------------------------------------------------------
// 四、UMoverDataModelBlueprintLibrary —— 纯工具库，弥补“结构体里不能写 UFUNCTION”
// -----------------------------------------------------------------------------
