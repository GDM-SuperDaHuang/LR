// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/GC/LrGCN_DamageNumber.h"

#include "Pawn/LrEnemyPawn.h"
#include "Tags/LrGameplayTags.h"
#include "UI/Component/LrWorldWidgetComponent.h"
#include "UI/Widget/Bar/WorldBar/LrWorldBarWidget.h"

// ULrGCN_DamageNumber::ULrGCN_DamageNumber()
// {
// 	// GameplayCueTag = FLrGameplayTags::Get().GameplayCue_Text_Damage;
// 	GameplayCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Text.Damage"));
// }

bool ULrGCN_DamageNumber::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	const FGameplayEffectContext* GameplayEffectContext = Parameters.EffectContext.Get();
	// 这个函数只会在客户端运行，绝对安全
	ALrEnemyPawn* Enemy = Cast<ALrEnemyPawn>(MyTarget);
	if (Enemy)
	{
		// 访问你 Pawn 上的组件
		ULrWorldWidgetComponent* LrWidgetComponent = Enemy->GetWorldWidgetComponent();
		if (LrWidgetComponent)
		{
			ULrWorldBarWidget* Widget = Cast<ULrWorldBarWidget>(LrWidgetComponent->GetUserWidgetObject());
			if (Widget)
			{
				// 调用我们之前写好的播放函数，传入从 Parameters 拿到的伤害值
				Widget->PlayDamageAnimation(Parameters.RawMagnitude);
			}
		}
	}
	// return Super::OnExecute_Implementation(MyTarget, Parameters);
	return true;
}
