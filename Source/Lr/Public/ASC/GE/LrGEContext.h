// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"

/**
 * GE 包扩展
 */
// USTRUCT(BlueprintType)
struct FLrGameplayEffectContext : public FGameplayEffectContext
{
	// GENERATED_BODY()

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
};

//注册到引擎
// template <>
// struct TStructOpsTypeTraits<FLrGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FLrGameplayEffectContext>
// {
// 	enum
// 	{
// 		WithNetSerializer = true,
// 		WithCopy = true,
// 	};
// };
