// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "LrGEContext.generated.h" // 必须最后 include

//设置 Flags |= Burn;
enum EDamageFlags : uint8
{
	None = 0,
	Critical = 1 << 0, // 00000001 是否暴击
	SpeedCut = 1 << 1, // 00000001 是否减速
	Vertigo = 1 << 2, // 00000010 眩晕
	Burn = 1 << 3, //  燃烧
	Frozen = 1 << 4, //  冻硬
	Poison = 1 << 5, //  中毒
	Repel = 1 << 6, //  击退
	Stiffness = 1 << 7, //  僵直
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
	uint8 Flags = 0;

	//////////
	/// 减速
	//////////
	UPROPERTY()
	float SpeedCutRate = 0.0f; //减速效率
	UPROPERTY()
	float SpeedCutDuration = 0.0f; //减速时间

	//////////
	/// 眩晕
	//////////
	UPROPERTY()
	float VertigoDuration = 0.0f; //眩晕时间

	//////////
	/// 燃烧
	//////////
	UPROPERTY()
	float BurnValue = 0.0f; //每秒燃烧
	UPROPERTY()
	float BurnDuration = 0.0f; //Burn时间

	//////////
	/// 冰冻
	//////////
	UPROPERTY()
	float FrozenDuration = 0.0f; //冰冻时间


	//////////
	/// 中毒
	//////////
	UPROPERTY()
	float PoisonValue = 0.0f; //每秒中毒
	UPROPERTY()
	float PoisonDuration = 0.0f; //中毒时间
	
	//////////
	/// 击退
	//////////
	UPROPERTY()
	FVector RepelVector = FVector::ZeroVector; //击退位置

	//////////
	/// 僵直
	//////////
	UPROPERTY()
	float StiffnessDuration = 0.0f; //僵直时间
	
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
		return (Flags & InFlag) != 0;
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
