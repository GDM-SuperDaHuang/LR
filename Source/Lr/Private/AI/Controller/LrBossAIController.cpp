// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controller/LrBossAIController.h"

void ALrBossAIController::InitializeAI()
{
	Super::InitializeAI();
	if (LrBehaviorTree)
	{
		RunBehaviorTree(LrBehaviorTree);
	}
}
