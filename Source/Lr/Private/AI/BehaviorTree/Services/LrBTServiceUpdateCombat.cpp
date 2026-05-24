// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Services/LrBTServiceUpdateCombat.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/LrAIStateComponent.h"
#include "Lr/Lr.h"
#include "Pawn/LrEnemyPawn.h"

ULrBTServiceUpdateCombat::ULrBTServiceUpdateCombat()
{
	// BTService 的默认执行间隔设为 0.2 秒
	// 太短浪费性能，太长则距离判断滞后
	Interval = 0.4f;
	// 随机偏移，避免大量AI同帧更新
	// RandomDeviation = 0.05f;
}

void ULrBTServiceUpdateCombat::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!AIController)
	{
		return;
	}

	ALrEnemyPawn* EnemyPawn = Cast<ALrEnemyPawn>(AIController->GetPawn());
	if (!EnemyPawn)
	{
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	ULrAIStateComponent* AIStateComp = EnemyPawn->GetAIStateComponent();
	if (!AIStateComp)
	{
		return;
	}

	// 死亡直接结束
	if (AIStateComp->GetAIState() == ELrAIState::Dead)
	{
		return;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(LrBBKeys::TargetActor));
	ELrAIState LrAIState = AIStateComp->GetAIState();
	// UE_LOG(LogTemp, Warning, TEXT("LrAIState : %d"), LrAIState);
	// 没有目标
	if (!Target)
	{
		SetAIState(OwnerComp, ELrAIState::Idle);
		// UE_LOG(LogTemp, Warning, TEXT("Idle : %d"), LrAIState);
		return;
	}

	const FVector SelfLocation = EnemyPawn->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();

	const float Distance = FVector::Dist(SelfLocation, TargetLocation);

	// 超出追击范围
	if (Distance > ChaseRange)
	{
		BB->ClearValue(LrBBKeys::TargetActor);
		SetAIState(OwnerComp, ELrAIState::Search);
		return;
	}

	// 是否有视线
	bool bHasLOS = true;
	if (bRequireLineOfSight)
	{
		bHasLOS = AIController->LineOfSightTo(Target);
	}

	// 无视线
	if (!bHasLOS)
	{
		SetAIState(OwnerComp, ELrAIState::Search);
		return;
	}

	// 进入战斗
	if (Distance <= AttackRange)
	{
		SetAIState(OwnerComp, ELrAIState::Combat);
	}
	else
	{
		SetAIState(OwnerComp, ELrAIState::Chase);
	}
}

void ULrBTServiceUpdateCombat::SetAIState(UBehaviorTreeComponent& OwnerComp, ELrAIState NewState)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return;
	}

	ALrEnemyPawn* EnemyPawn = Cast<ALrEnemyPawn>(AIController->GetPawn());
	if (!EnemyPawn)
	{
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	ULrAIStateComponent* AIStateComp = EnemyPawn->GetAIStateComponent();
	if (!AIStateComp)
	{
		return;
	}

	const uint8 CurrentBBState = BB->GetValueAsEnum(LrBBKeys::AIState);

	// Blackboard 和目标状态相同
	if (CurrentBBState == static_cast<uint8>(NewState))
	{
		return;
	}

	// 更新组件状态
	AIStateComp->SetAIState(NewState);
	// 更新 Blackboard
	BB->SetValueAsEnum(LrBBKeys::AIState, static_cast<uint8>(NewState));

	// UE_LOG(LogTemp, Warning, TEXT("AI State Changed -> %d"), static_cast<uint8>(NewState));
}
