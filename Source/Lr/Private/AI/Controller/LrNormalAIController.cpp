// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controller/LrNormalAIController.h"

void ALrNormalAIController::InitializeAI()
{
	Super::InitializeAI();
	if (LrBehaviorTree)
	{
		RunBehaviorTree(LrBehaviorTree);
	}
}
