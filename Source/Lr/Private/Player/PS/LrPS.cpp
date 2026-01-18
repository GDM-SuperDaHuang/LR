// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PS/LrPS.h"

#include "ASC/LrASC.h"
#include "ASC/AS/LrAS.h"

ALrPS::ALrPS()
{
	NetUpdateFrequency = 100.0f;
	// 这里ASC最大扫描和UAureAttributeSet进行绑定
	LrASC = CreateDefaultSubobject<ULrASC>("AbilitySystemComponent");
	LrASC->SetIsReplicated(true); //复制
	LrASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	LrAS = CreateDefaultSubobject<ULrAS>("AttributeSet");

}

void ALrPS::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	/**
	 * 持久化操作
	 * 第一个参数 AAurePlayerState是声明属性的类名。
	 * 第二个参数 Level是要复制的变量名。 
	 * 使用getLevel就能获取到最新的Level
	 */
	// DOREPLIFETIME(AAurePlayerState, Level);
	// DOREPLIFETIME(AAurePlayerState, XP);
	// DOREPLIFETIME(AAurePlayerState, AttributePoints);
	// DOREPLIFETIME(AAurePlayerState, SpellPoints);
}

UAbilitySystemComponent* ALrPS::GetAbilitySystemComponent() const
{
	return LrASC;
}
