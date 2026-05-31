// Fill out your copyright notice in the Description page of Project Settings.
/**
	*GameplayEffect
		↓
	Modifier计算
		↓
	PreAttributeChange // 里面禁止SetHP(...)，否则无限递归
		↓
	属性真正写入
		↓
	PostGameplayEffectExecute //GameplayEffect 已经执行完毕,Modifier 已经落地
		↓
	PostAttributeChange
		↓
	网络复制
		↓
	OnRep
		↓
	UI刷新
*/
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

class ALrPawnBase;
class ULrBuffDA;
struct FLrGameplayEffectContext;
DECLARE_DELEGATE_RetVal(FGameplayAttribute, FAttributeSignature);

// 方式三 定义成员函数指针
// template <class T>
// using FAttributeFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;
typedef TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr FAttributeFuncPtr;


USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()
	FEffectProperties()
	{
	}

	FGameplayEffectContextHandle EffectContextHandle; // 效果上下文句柄，存储效果元数据

	// 源（施加效果的对象）相关信息
	UPROPERTY()
	UAbilitySystemComponent* SourceASC; // 来源的能力系统组件
	UPROPERTY()
	AActor* SourceAvatarActor; // 来源的Avatar角色
	UPROPERTY()
	AController* SourceController; // 来源的控制器
	UPROPERTY()
	ALrPawnBase* SourceCharacter; // 来源的角色

	// 目标（接收效果的对象）相关信息
	UPROPERTY()
	UAbilitySystemComponent* TargetASC; // 目标的能力系统组件
	UPROPERTY()
	AActor* TargetAvatarActor; // 目标的Avatar角色
	UPROPERTY()
	AController* TargetController; // 目标的控制器
	UPROPERTY()
	ALrPawnBase* TargetCharacter; // 目标的角色
};

/**
 * 
 */
UCLASS()
class LR_API ULrAS : public UAttributeSet
{
	GENERATED_BODY()

public:
	ULrAS();

	UPROPERTY(EditDefaultsOnly, Category = "DA")
	TObjectPtr<ULrBuffDA> LrBuffDA;

	// 网络复制支持：定义哪些属性需要复制到客户端
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;;

	// 执行时机是属性值即将被修改但尚未实际更新到 FGameplayAttributeData 之前
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	// 效果执行后的回调（后处理阶段）
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	TMap<FGameplayTag, FAttributeFuncPtr> TagsASMap; //<标签,Get属性函数指针>
	TMap<FGameplayTag, FGameplayTag> TagsASMaxTags; //<标签,Max标签>

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


	/*
	* Damage Pipeline
	*/
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData IncomingDamage; //伤害
	ATTRIBUTE_ACCESSORS(ULrAS, IncomingDamage)

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData IncomingHeal; //治疗
	ATTRIBUTE_ACCESSORS(ULrAS, IncomingHeal)

	/*
	 * Combat
	 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(ULrAS, Defense)

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(ULrAS, Shield)

	UFUNCTION()
	void OnRepHp(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRepMp(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRepEndurance(const FGameplayAttributeData& OldValue) const;

protected:
	void ApplyDebuff(const FLrGameplayEffectContext& LrGEContext, const FEffectProperties& Props);
	void SetEffectProperties(const struct FGameplayEffectModCallbackData& Data, FEffectProperties& props) const;
};
