// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controller/LrAIControllerBase.h"

#include "NavigationSystem.h"
#include "Nav3DData.h"
#include "Pathfinding/Core/Nav3DPath.h"
#include "Pathfinding/Core/Nav3DPathCoordinator.h"
#include "Pathfinding/Core/Nav3DPathingTypes.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Component/LrPatrolRouteComponent.h"
#include "Mover/Nav/LrNavMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "Pawn/LrEnemyPawn.h"
#include "Perception/AIPerceptionComponent.h"

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
	int32 UserFlags = MoveRequest.GetUserFlags();
	if (UserFlags == 0)
	{
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		const ANavigationData* RecastNavMesh = NavSys
			                                       ? NavSys->GetNavDataForAgentName(TEXT("Ground"))
			                                       : nullptr;
		if (RecastNavMesh)
		{
			Query.NavData = const_cast<ANavigationData*>(RecastNavMesh);
			Query.NavAgentProperties = static_cast<FNavAgentProperties>(RecastNavMesh->GetConfig());
			Query.QueryFilter = MoveRequest.GetNavigationFilter()
				                    ? UNavigationQueryFilter::GetQueryFilter(
					                    *RecastNavMesh,
					                    this,
					                    MoveRequest.GetNavigationFilter())
				                    : RecastNavMesh->GetDefaultQueryFilter();
			Super::FindPathForMoveRequest(MoveRequest, Query, OutPath);

			// UE_LOG(LogTemp, Warning, TEXT("[%s] Ground path query: NavData=%s, PathValid=%s, Ready=%s, Points=%d, Filter=%s"),
			// 	*GetNameSafe(GetPawn()),
			// 	*GetNameSafe(RecastNavMesh),
			// 	OutPath.IsValid() && OutPath->IsValid() ? TEXT("TRUE") : TEXT("FALSE"),
			// 	OutPath.IsValid() && OutPath->IsReady() ? TEXT("TRUE") : TEXT("FALSE"),
			// 	OutPath.IsValid() ? OutPath->GetPathPoints().Num() : 0,
			// 	Query.QueryFilter.IsValid() ? TEXT("TRUE") : TEXT("FALSE"));
		}
		return;
	}

	// Name=Nav3DData-Fly
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

	const ENavigationQueryResult::Type Result = FNav3DPathCoordinator::FindPath(*Nav3DPath, Nav3DRequest);
	if (Result != ENavigationQueryResult::Success || !Nav3DPath->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Nav3D path failed: Result=%d, Points=%d"), *GetNameSafe(GetPawn()), static_cast<int32>(Result), Nav3DPath->GetPathPoints().Num());
		OutPath.Reset();
	}
}


void ALrAIControllerBase::MoverToTarget(const FVector& TargetLocation, int32 MoveMode)
{
	MoveState = EMoveState::Start;
	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(TargetLocation);
	MoveReq.SetAcceptanceRadius(20.f);
	MoveReq.SetUsePathfinding(true);
	MoveReq.SetProjectGoalLocation(false);
	MoveReq.SetRequireNavigableEndLocation(false);
	MoveReq.SetAllowPartialPath(false);
	MoveReq.SetUserFlags(MoveMode);
	const FPathFollowingRequestResult Result = MoveTo(MoveReq);
	if (Result.Code != EPathFollowingRequestResult::RequestSuccessful && Result.Code != EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Nav3D MoveTo failed for %s (Code=%d)"),
		       *GetNameSafe(GetPawn()), *TargetLocation.ToString(), static_cast<int32>(Result.Code));
	}
	// UE_LOG(LogTemp, Warning, TEXT("[%s] MoveTo result: Code=%d, MoveStatus=%d, NavMove=%s"),
	// 	*GetNameSafe(GetPawn()),
	// 	static_cast<int32>(Result.Code),
	// 	GetMoveStatus(),
	// 	GetPawn() && GetPawn()->FindComponentByClass<ULrNavMovementComponent>()
	// 		? *GetNameSafe(Cast<UObject>(GetPawn()->FindComponentByClass<ULrNavMovementComponent>()))
	// 		: TEXT("NONE"));
}

void ALrAIControllerBase::MoveToPlayer(AActor* Player, int32 MoverMode)
{
	MoveState = EMoveState::Start;
	UPathFollowingComponent* FollowingComponent = GetPathFollowingComponent();
	if (FollowingComponent && FollowingComponent->GetStatus() != EPathFollowingStatus::Idle)
	{
		FollowingComponent->AbortMove(*this, FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest
		                              , FAIRequestID::CurrentRequest, EPathFollowingVelocityMode::Keep);
	}

	FAIMoveRequest MoveReq(Player);
	MoveReq.SetUsePathfinding(true);
	MoveReq.SetAllowPartialPath(true);
	MoveReq.SetNavigationFilter(DefaultNavigationFilterClass);
	MoveReq.SetAcceptanceRadius(AcceptanceRadius);
	MoveReq.SetReachTestIncludesAgentRadius(true);
	MoveReq.SetCanStrafe(true);
	MoveReq.SetUserFlags(MoverMode);
	MoveTo(MoveReq);
	// abort active movement to keep only one request running
	// MoveToActor(Player, 50.f);
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
	if (GetPathFollowingComponent())
	{
		GetPathFollowingComponent()->UpdateCachedComponents();
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
}

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
