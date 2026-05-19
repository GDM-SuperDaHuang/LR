// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controller/LrAIControllerBase.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/LrAIStateComponent.h"
#include "Lr/Lr.h"
#include "Navigation/PathFollowingComponent.h"
#include "Pawn/LrEnemyPawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

ALrAIControllerBase::ALrAIControllerBase()
{

	LrBlackboardComponent =  CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	// LrBehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	// 创建感知组件，负责驱动所有配置的 AI 感官（视觉、听觉等）
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

	// 创建视觉感知配置，设定视野参数
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2000.f;                  // 完全可见范围（2000cm = 20m）
	SightConfig->LoseSightRadius = 2500.f;              // 丢失视野范围（2500cm = 25m，提供 5m 的滞后区防抖动）
	SightConfig->PeripheralVisionAngleDegrees = 90.f;   // 视野半角 90 度，即全视野 180 度
	SightConfig->SetMaxAge(5.f);                        // 目标丢失后记忆 5 秒

	// 启用对所有阵营的检测（敌人、中立、友方均可见）
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// 将视觉配置注册到感知组件，并设置为主导感官
	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());
	
	check(GetPathFollowingComponent());
}

FGenericTeamId ALrAIControllerBase::GetGenericTeamId() const
{
	return FGenericTeamId(TeamID);
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
	// UBehaviorTree* BehaviorTree = LrBehaviorTreeComponent->GetRootTree()
	if (TObjectPtr<UBlackboardData> BlackboardData = LrBlackboardComponent->GetBlackboardAsset())
	{
		UBlackboardComponent* BBPtr = LrBlackboardComponent;
		UseBlackboard(BlackboardData,BBPtr);
	}
	
	Blackboard->SetValueAsVector( LrBBKeys::HomeLocation, Enemy->GetHomeLocation());
	RunBehaviorTree(Enemy->GetBehaviorTree());

	// 绑定感知回调：当感知系统检测到或丢失目标时，触发 OnTargetDetected
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this,&ALrAIControllerBase::OnTargetDetected);
}

void ALrAIControllerBase::InitializeAI()
{
}

void ALrAIControllerBase::ShutdownAI()
{
	StopMovement();
	BrainComponent->StopLogic(TEXT("Shutdown"));
}

void ALrAIControllerBase::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{

	if (!Stimulus.WasSuccessfullySensed())
	{
		return;
	}
	
	ALrEnemyPawn* SelfPawn = Cast<ALrEnemyPawn>(GetPawn());
	if (!SelfPawn)
	{
		return;
	}

	FGenericTeamId SelfTeamId = GetGenericTeamId();
	IGenericTeamAgentInterface* OtherTeam = Cast<IGenericTeamAgentInterface>(Actor);
	if ( !OtherTeam)
	{
		return;
	}

	// 是否是敌人
	const bool bIsEnemy = SelfTeamId!= OtherTeam->GetGenericTeamId();
	if (!bIsEnemy)
	{
		return;
	}

	UBlackboardComponent* BB = GetBlackboardComponent();
	ULrAIStateComponent* AIState = SelfPawn->GetAIStateComponent();
	if (!BB || !AIState)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		BB->SetValueAsObject( LrBBKeys::TargetActor, Actor);
		BB->SetValueAsVector( LrBBKeys::LastKnownLocation, Actor->GetActorLocation());
		AIState->SetAIState( ELrAIState::Chase);
		BB->SetValueAsEnum( LrBBKeys::AIState, (uint8)ELrAIState::Chase);
	}
	else
	{
		// 丢失目标
		BB->ClearValue( LrBBKeys::TargetActor);
		AIState->SetAIState( ELrAIState::Search);
		BB->SetValueAsEnum( LrBBKeys::AIState, (uint8)ELrAIState::Search);
	}
}

void ALrAIControllerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// if (!bIsChasing)
	// {
	// 	return;
	// }
	//
	// if (!CurrentTarget)
	// {
	// 	StopChase();
	// 	return;
	// }
	AActor* TargetActor = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(LrBBKeys::TargetActor));
	if (!TargetActor)
	{
		return;
	}
	// MoveToActor(
	// 	TargetActor,
	// 	AcceptanceRadius,
	// 	true,
	// 	true,
	// 	true,
	// 	nullptr,
	// 	true);

	FAIMoveRequest Request;
	Request.SetGoalActor(TargetActor);
	Request.SetAcceptanceRadius(100.f);

	FNavPathSharedPtr Path;
	FPathFollowingRequestResult PathFollowingRequestResult = MoveTo(Request, &Path);
	UE_LOG(LogTemp, Warning,TEXT("MoveTo Result: %d"),(int32)PathFollowingRequestResult);

	UNavigationSystemV1* NavSys =FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	UE_LOG(LogTemp, Warning, TEXT("NavSys=%s"), *GetNameSafe(NavSys));
	

	FNavLocation NavLoc;
	bool bOnNavMesh =NavSys->ProjectPointToNavigation(TargetActor->GetActorLocation(),NavLoc);
	UE_LOG(LogTemp, Warning, TEXT("OnNavMesh=%d"), bOnNavMesh);
}

// void ALrAIControllerBase::StartChase(AActor* InTarget)
// {
// 	if (!InTarget)
// 	{
// 		return;
// 	}
// 	CurrentTarget = InTarget;
// 	bIsChasing = true;
// }
//
// void ALrAIControllerBase::StopChase()
// {
// 	bIsChasing = false;
// 	StopMovement();
// 	CurrentTarget = nullptr;
// }
