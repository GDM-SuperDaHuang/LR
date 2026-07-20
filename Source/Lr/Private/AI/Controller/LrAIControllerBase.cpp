// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controller/LrAIControllerBase.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Component/LrPatrolRouteComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Pawn/LrEnemyPawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"

ALrAIControllerBase::ALrAIControllerBase()
{
	// LrBlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	// LrBehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	// 创建感知组件，负责驱动所有配置的 AI 感官（视觉、听觉等）
	// PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

	// 创建视觉感知配置，设定视野参数
	// SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	// SightConfig->SightRadius = 2000.f; // 完全可见范围（2000cm = 20m）
	// SightConfig->LoseSightRadius = 2500.f; // 丢失视野范围（2500cm = 25m，提供 5m 的滞后区防抖动）
	// SightConfig->PeripheralVisionAngleDegrees = 150.f; // 视野半角 90 度，即全视野 180 度
	// SightConfig->SetMaxAge(5.f); // 目标丢失后记忆 5 秒
	//
	// // 启用对所有阵营的检测（敌人、中立、友方均可见）
	// SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	// SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	// SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// 将视觉配置注册到感知组件，并设置为主导感官
	// PerceptionComponent->ConfigureSense(*SightConfig);
	// PerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());

	check(GetPathFollowingComponent());
}

FGenericTeamId ALrAIControllerBase::GetGenericTeamId() const
{
	return FGenericTeamId(TeamID);
}


void ALrAIControllerBase::MoverToTarget(const FVector& TargetLocation)
{
	MoveState = EMoveState::Start;
	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(TargetLocation);
	MoveReq.SetAcceptanceRadius(20.f);
	FAIRequestID FaiRequestID = MoveTo(MoveReq).MoveId;
}

void ALrAIControllerBase::MoveToPlayer(AActor* Player)
{
	MoveState = EMoveState::Start;
	MoveToActor(Player, 50.f);
}


void ALrAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	InitializeAI();
	const ALrEnemyPawn* Enemy = Cast<ALrEnemyPawn>(InPawn);
	if (!Enemy)
	{
		return;
	}

	//生成巡逻点
	ULrPatrolRouteComponent* Patrol = Enemy->GetPatrolRoute();
	if (Patrol)
	{
		FVector Home = Enemy->GetActorLocation();
		Patrol->SetHomeLocation(Home);
		// 生成半径500，6个点的巡逻圈
		Patrol->GenerateCircle(500.f, 6);
	}

	// UBehaviorTree* BT = Enemy->GetBehaviorTree();
	// if (!BT)
	// {
	// 	return;
	// }
	// UBlackboardComponent* BBPtr = Blackboard;
	// UseBlackboard(BT->BlackboardAsset, BBPtr);
	//
	// Blackboard->SetValueAsVector(LrBBKeys::HomeLocation, Enemy->GetHomeLocation());
	// RunBehaviorTree(Enemy->GetBehaviorTree());
	// BBPtr->SetValueAsEnum(LrBBKeys::AIState, static_cast<uint8>(ELrAIState::Idle));

	// 先解开再重新绑定感知回调：当感知系统检测到或丢失目标时，触发 OnTargetDetected
	// PerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(this, &ALrAIControllerBase::OnTargetDetected);
	// PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ALrAIControllerBase::OnTargetDetected);
}

// void ALrAIControllerBase::OnPossess(APawn* InPawn)
// {
// 	Super::OnPossess(InPawn);
// 	InitializeAI();
// 	const ALrEnemyPawn* Enemy = Cast<ALrEnemyPawn>(InPawn);
// 	if (!Enemy)
// 	{
// 		return;
// 	}
//
// 	//生成巡逻点
// 	ULrPatrolRouteComponent* Patrol = Enemy->GetPatrolRoute();
// 	if (Patrol)
// 	{
// 		FVector Home = Enemy->GetActorLocation();
// 		Patrol->SetHomeLocation(Home);
// 		// 生成半径500，6个点的巡逻圈
// 		Patrol->GenerateCircle(500.f, 6);
// 	}
//
// 	UBehaviorTree* BT = Enemy->GetBehaviorTree();
// 	if (!BT)
// 	{
// 		return;
// 	}
// 	UBlackboardComponent* BBPtr = Blackboard;
// 	UseBlackboard(BT->BlackboardAsset, BBPtr);
// 	Blackboard->SetValueAsVector(LrBBKeys::HomeLocation, Enemy->GetHomeLocation());
// 	RunBehaviorTree(Enemy->GetBehaviorTree());
// 	BBPtr->SetValueAsEnum(LrBBKeys::AIState, static_cast<uint8>(ELrAIState::Idle));
//
// 	// 先解开再重新绑定感知回调：当感知系统检测到或丢失目标时，触发 OnTargetDetected
// 	PerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(this, &ALrAIControllerBase::OnTargetDetected);
// 	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ALrAIControllerBase::OnTargetDetected);
// }

void ALrAIControllerBase::InitializeAI()
{
}

void ALrAIControllerBase::ShutdownAI()
{
	StopMovement();
	BrainComponent->StopLogic(TEXT("Shutdown"));
}

void ALrAIControllerBase::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	// if (Result == EPathFollowingResult::Success)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("到达目标"));
	// }
	// else if (Result == EPathFollowingResult::Aborted)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("移动被取消"));
	// }
	// else if (Result == EPathFollowingResult::Blocked)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("被阻挡"));
	// }
	MoveState = EMoveState::Finish;
}

void ALrAIControllerBase::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	// if (!Actor)
	// {
	// 	return;
	// }
	//
	// if (Stimulus.WasSuccessfullySensed())
	// {
	// 	BB->SetValueAsObject(LrBBKeys::TargetActor, Actor);
	// 	BB->SetValueAsVector(LrBBKeys::LastKnownLocation, Actor->GetActorLocation());
	// }
	// else
	// {
	// 	BB->ClearValue(LrBBKeys::TargetActor);
	// }
}

// void ALrAIControllerBase::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
// {
// 	if (!Actor)
// 	{
// 		return;
// 	}
//
// 	UBlackboardComponent* BB = GetBlackboardComponent();
// 	if (!BB)
// 	{
// 		return;
// 	}
// 	if (Stimulus.WasSuccessfullySensed())
// 	{
// 		BB->SetValueAsObject(LrBBKeys::TargetActor, Actor);
// 		BB->SetValueAsVector(LrBBKeys::LastKnownLocation, Actor->GetActorLocation());
// 	}
// 	else
// 	{
// 		BB->ClearValue(LrBBKeys::TargetActor);
// 	}
// }

// void ALrAIControllerBase::SetTargetActor(AActor* Target)
// {
// 	UBlackboardComponent* BB = GetBlackboardComponent();
// 	if (!BB)
// 	{
// 		return;
// 	}
// 	BB->SetValueAsObject(LrBBKeys::TargetActor,Target);
// 	if (Target)
// 	{
// 		BB->SetValueAsVector(LrBBKeys::LastKnownLocation, Target->GetActorLocation());
// 	}
// }

// void ALrAIControllerBase::ClearTargetActor()
// {
// 	if (UBlackboardComponent* BB = GetBlackboardComponent())
// 	{
// 		BB->ClearValue(LrBBKeys::TargetActor);
// 	}
// }
