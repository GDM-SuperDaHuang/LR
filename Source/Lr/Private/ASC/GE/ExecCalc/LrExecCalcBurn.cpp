// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GE/ExecCalc/LrExecCalcBurn.h"

#include "ASC/AS/LrAS.h"
#include "Tags/LrGameplayTags.h"


struct FLrBurnStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);

	FLrBurnStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(ULrAS, Defense, Target, false);
	}
};

static const FLrBurnStatics& DamageStatics()
{
	static FLrBurnStatics DStatices;
	return DStatices;
}

ULrExecCalcBurn::ULrExecCalcBurn()
{
	RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
}

void ULrExecCalcBurn::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

	/**
	 * GAS 传进来的执行参数
	 * 包含：
	 * Source ASC
	 * Target ASC
	 * GameplayEffectSpec
	 * Captured Attributes
	 * Tags
	 */
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec(); //模板
	FLrGameplayTags LrGameplayTags = FLrGameplayTags::Get();
	//取出运行时伤害值
	float Damage = Spec.GetSetByCallerMagnitude(LrGameplayTags.Damage_Base, false, 0.f);

	// 当前燃烧层数
	int32 StackCount = Spec.GetStackCount();
	// 每层增加伤害
	Damage *= StackCount;

	//防御
	float Defense = 0.f;
	FAggregatorEvaluateParameters EvaluateParameters;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseDef, EvaluateParameters, Defense);

	Defense = FMath::Clamp(Defense, 0.f, 100.f);
	Damage = Damage - Defense;

	if (Damage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			ULrAS::GetIncomingDamageAttribute(),
			EGameplayModOp::Additive,
			Damage));
	}
}
