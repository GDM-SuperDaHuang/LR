// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GE/ExecCalc/LrExecCalcDamage.h"

#include "ASC/AS/LrAS.h"
#include "ASC/GE/LrGEContext.h"
#include "Data/LrBuffDA.h"
#include "Lib/LrCommonLibrary.h"
#include "Lr/Lr.h"
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

ULrExecCalcDamage::ULrExecCalcDamage()
{
	RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
}

void ULrExecCalcDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
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

	//------------------------------------
	// 层数
	//------------------------------------
	// 当前燃烧层数
	int32 StackCount = FMath::Max(Spec.GetStackCount(), 1);;
	// 每层增加伤害
	// Damage *= StackCount;
	uint8 Flags = Spec.GetSetByCallerMagnitude(LrGEKeys::Flags, false);
	float SpeedCutRate = Spec.GetSetByCallerMagnitude(LrGEKeys::SpeedCutRate, false);
	float DamageValue = Spec.GetSetByCallerMagnitude(LrGEKeys::DamageValue, false);
	Damage += DamageValue;

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
