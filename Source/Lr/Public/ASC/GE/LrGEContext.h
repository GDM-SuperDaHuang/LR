// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "LrGEContext.generated.h" // 必须最后 include

class UGameplayEffect;

//设置 Flags |= Burn;
UENUM(BlueprintType)
enum class EDamageFlags : uint8
{
	None = 0 UMETA(Hidden),
	Critical = 1 << 0,
	SpeedCut = 1 << 1,
	Vertigo = 1 << 2,
	Burn = 1 << 3,
	Frozen = 1 << 4,
	Poison = 1 << 5,
	Repel = 1 << 6,
	Stiffness = 1 << 7,
};

ENUM_CLASS_FLAGS(EDamageFlags)

USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()

	FDamageEffectParams()
	{
	}

	UPROPERTY()
	EDamageFlags Flags = EDamageFlags::None;
	//////////
	/// 减速
	//////////
	UPROPERTY()
	float SpeedCutRate = 0.0f; //减速效率
	UPROPERTY()
	float Duration = 0.0f; //持续时间
	UPROPERTY()
	float DamageValue = 0.0f; //每秒伤害
	//////////
	/// 击退
	//////////
	UPROPERTY()
	FVector RepelVector = FVector::ZeroVector; //击退位置

	//添加标志
	FORCEINLINE void AddFlag(EDamageFlags InFlag)
	{
		Flags |= InFlag;
	}
	//移除标志
	FORCEINLINE void RemoveFlag(EDamageFlags InFlag)
	{
		Flags &= ~InFlag;
	}
};

/**
 * GE 包扩展
 */
USTRUCT()
struct FLrGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	UPROPERTY()
	EDamageFlags Flags = EDamageFlags::None;

	//////////
	/// 减速
	//////////
	UPROPERTY()
	float SpeedCutRate = 0.0f; //减速效率

	UPROPERTY()
	float Duration = 0.0f; //持续时间

	UPROPERTY()
	float DamageValue = 0.0f; //每秒伤害
	//////////
	/// 击退
	//////////
	UPROPERTY()
	FVector RepelVector = FVector::ZeroVector; //击退位置
public:
	/**
	 * 需要通过网络复制（服务器→客户端）时，请按我规定的格式打包/解包，否则默认序列化会漏掉我扩展的成员变量
	 */
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);

	/** Creates a copy of this context, used to duplicate for later modifications */
	virtual FLrGameplayEffectContext* Duplicate() const
	{
		FLrGameplayEffectContext* NewContext = new FLrGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual UScriptStruct* GetScriptObject() const
	{
		return FLrGameplayEffectContext::StaticStruct();
	}

	//是否命中
	FORCEINLINE bool HasFlag(EDamageFlags InFlag) const
	{
		return EnumHasAnyFlags(Flags, InFlag);
	}

	FORCEINLINE bool HasFlag(EDamageFlags InFlag1, EDamageFlags InFlag2) const
	{
		return EnumHasAnyFlags(InFlag1, InFlag2);
	}

	//添加标志
	FORCEINLINE void AddFlag(EDamageFlags InFlag)
	{
		Flags |= InFlag;
	}

	//移除标志
	FORCEINLINE void RemoveFlag(EDamageFlags InFlag)
	{
		Flags &= ~InFlag;
	}
};

// 注册到引擎
template <>
struct TStructOpsTypeTraits<FLrGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FLrGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true,
	};
};
