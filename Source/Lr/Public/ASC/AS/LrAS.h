// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "LrAS.generated.h"


//生成getter、setter、Init 函数
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DELEGATE_RetVal(FGameplayAttribute, FAttributeSignature);

// 方式三 定义成员函数指针
// template <class T>
// using FAttributeFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

typedef TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr FAttributeFuncPtr;

/**
 * 
 */
UCLASS()
class LR_API ULrAS : public UAttributeSet
{
	GENERATED_BODY()

public:
	ULrAS();

	// 网络复制支持：定义哪些属性需要复制到客户端
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;;

	// 执行时机是属性值即将被修改但尚未实际更新到 FGameplayAttributeData 之前
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	// 效果执行后的回调（后处理阶段）
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	TMap<FGameplayTag, FAttributeFuncPtr> TagsASMap;
	// TMap<FGameplayTag, FAttributeFuncPtr<FGameplayAttribute()>> TagsASMap;

	/*
	 * 主要属性
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRepHp, Category="primary Attributes")
	FGameplayAttributeData HP;
	ATTRIBUTE_ACCESSORS(ULrAS, HP); //生成getter、setter函数


	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRepHp, Category="primary Attributes")
	FGameplayAttributeData MaxHP;
	ATTRIBUTE_ACCESSORS(ULrAS, MaxHP); 
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRepMp, Category="primary Attributes")
	FGameplayAttributeData MP;
	ATTRIBUTE_ACCESSORS(ULrAS, MP);
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRepMp, Category="primary Attributes")
	FGameplayAttributeData MaxMP;
	ATTRIBUTE_ACCESSORS(ULrAS, MaxMP); 

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRepEndurance, Category="primary Attributes")
	FGameplayAttributeData Endurance;
	ATTRIBUTE_ACCESSORS(ULrAS, Endurance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRepEndurance, Category="primary Attributes")
	FGameplayAttributeData MaxEndurance;
	ATTRIBUTE_ACCESSORS(ULrAS, MaxEndurance); 

	// UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRepVigor, Category="primary Attributes")
	// FGameplayAttributeData Vigor;
	// ATTRIBUTE_ACCESSORS(ULrAttributeSet, Vigor); //生成getter、setter函数

	UFUNCTION()
	void OnRepHp(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRepMp(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRepEndurance(const FGameplayAttributeData& OldValue) const;
};
