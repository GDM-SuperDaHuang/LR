// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Services/LrBTServiceUpdateCombat.h"

#include "BehaviorTree/BlackboardComponent.h"

ULrBTServiceUpdateCombat::ULrBTServiceUpdateCombat()
{
	// BTService 的默认执行间隔设为 0.2 秒
	// 太短浪费性能，太长则距离判断滞后
	Interval = 0.2f;
}

void ULrBTServiceUpdateCombat::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	// if (!BB)
	// {
	// 	return;
	// }
	//
	// // 获取 AI 自身控制的 Pawn 和当前感知到的目标 Actor
	// APawn* SelfPawn = OwnerComp.GetAIOwner()->GetPawn();
	// AActor* Target = Cast<AActor>(BB->GetValueAsObject(LrBBKeys::TargetActor));
	//
	// // 没有自身或目标时，标记为非战斗状态
	// if (!SelfPawn || !Target)
	// {
	// 	BB->SetValueAsBool(LrBBKeys::CombatState, false);
	// 	return;
	// }
	//
	// // 计算自身与目标之间的距离
	// const float Distance = FVector::Distance(SelfPawn->GetActorLocation(), Target->GetActorLocation());
	//
	// // 写入黑板：行为树的 Decorator 可读取 DistanceToTarget 做阈值判断
	// BB->SetValueAsFloat(TEXT("DistanceToTarget"), Distance);
	// // 写入黑板：距离 <= 攻击范围时为 true，用于切换攻击分支
	// BB->SetValueAsBool(LrBBKeys::CombatState, Distance <= AttackRange);
}