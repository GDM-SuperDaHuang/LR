// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/LrAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Lr/Lr.h"
#include "Pawn/LrEnemyPawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

ALrAIController::ALrAIController()
{
	BBComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BBComponent"));

	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2000.f;
	SightConfig->LoseSightRadius = 2500.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;
	SightConfig->SetMaxAge(5.f);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());
}

void ALrAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	const ALrEnemyPawn* Enemy = Cast<ALrEnemyPawn>(InPawn);
	if (!Enemy)
	{
		return;
	}

	UBehaviorTree* BT = Enemy->GetBehaviorTree();
	if (BT)
	{
		RunBehaviorTree(BT);
		BBComponent = GetBlackboardComponent();
	}

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ALrAIController::OnTargetDetected);
}

void ALrAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	if (!BBComponent)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		BBComponent->SetValueAsObject(LrBBKeys::TargetActor, Actor);
	}
	else
	{
		BBComponent->ClearValue(LrBBKeys::TargetActor);
	}
}