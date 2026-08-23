// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controller/LrAIControllerBase.h"

#include "NavigationSystem.h"
#include "Nav3DData.h"
#include "Pathfinding/Core/Nav3DPath.h"
#include "Pathfinding/Core/Nav3DPathCoordinator.h"
#include "Pathfinding/Core/Nav3DPathingTypes.h"
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
	UPathFollowingComponent* PathFollowing = GetPathFollowingComponent();

	UE_LOG(LogTemp, Warning, TEXT("PathFollowing=%s"), *GetNameSafe(PathFollowing));
}

FGenericTeamId ALrAIControllerBase::GetGenericTeamId() const
{
	return FGenericTeamId(TeamID);
}

void ALrAIControllerBase::FindPathForMoveRequest(const FAIMoveRequest& MoveRequest, FPathFindingQuery& Query, FNavPathSharedPtr& OutPath) const
{
	OutPath.Reset();

	const ANav3DData* Nav3DData = Cast<ANav3DData>(Query.NavData.Get());
	if (!Nav3DData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Move request has no Nav3DData"), *GetNameSafe(GetPawn()));
		return;
	}

	FNav3DPathingRequest Nav3DRequest;
	Nav3DRequest.StartLocation = Query.StartLocation;
	Nav3DRequest.EndLocation = Query.EndLocation;
	Nav3DRequest.NavData = Nav3DData;
	Nav3DRequest.AgentProperties = GetNavAgentPropertiesRef();
	Nav3DRequest.LogVerbosity = ENav3DPathingLogVerbosity::Silent;

	OutPath = Nav3DData->CreatePathInstance<FNav3DPath>(Query);
	FNav3DPath* Nav3DPath = OutPath.IsValid() ? OutPath->CastPath<FNav3DPath>() : nullptr;
	if (!Nav3DPath)
	{
		OutPath.Reset();
		return;
	}

	const ENavigationQueryResult::Type Result = FNav3DPathCoordinator::FindPath(
		*Nav3DPath,
		Nav3DRequest);
	if (Result != ENavigationQueryResult::Success || !Nav3DPath->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Nav3D path failed: Result=%d, Points=%d"),
		       *GetNameSafe(GetPawn()), static_cast<int32>(Result), Nav3DPath->GetPathPoints().Num());
		OutPath.Reset();
	}
}


void ALrAIControllerBase::MoverToTarget(const FVector& TargetLocation)
{
	MoveState = EMoveState::Start;
	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(TargetLocation);
	MoveReq.SetAcceptanceRadius(20.f);
	MoveReq.SetUsePathfinding(true);
	MoveReq.SetProjectGoalLocation(false);
	MoveReq.SetRequireNavigableEndLocation(false);
	MoveReq.SetAllowPartialPath(false);

	const FPathFollowingRequestResult Result = MoveTo(MoveReq);
	if (Result.Code != EPathFollowingRequestResult::RequestSuccessful &&
		Result.Code != EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Nav3D MoveTo failed for %s (Code=%d)"),
		       *GetNameSafe(GetPawn()), *TargetLocation.ToString(), static_cast<int32>(Result.Code));
	}
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
	UE_LOG(LogTemp, Verbose, TEXT("[%s] Move completed: Result=%d"),
	       *GetNameSafe(GetPawn()), static_cast<int32>(Result));
	// MoveState = Result == EPathFollowingResult::Success
	// 	            ? EMoveState::Finish
	// 	            : EMoveState::None;

	MoveState = EMoveState::Finish;
}
