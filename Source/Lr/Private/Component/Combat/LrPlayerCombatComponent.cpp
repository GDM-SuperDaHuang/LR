// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Combat/LrPlayerCombatComponent.h"

#include "Pawn/LrEnemyPawn.h"
#include "Pawn/LrHeroPawn.h"
#include "Player/LrPlayerController.h"

// Sets default values for this component's properties
ULrPlayerCombatComponent::ULrPlayerCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TWeakObjectPtr<ALrPawnBase> ULrPlayerCombatComponent::GetNearestPawnToCursor()
{
	TCopyQualifiersFromTo_T<UObject, ALrHeroPawn>* LrHeroPawn = Cast<ALrHeroPawn>(GetOuter());
	ALrPlayerController* PC = Cast<ALrPlayerController>(LrHeroPawn->GetController());
	ALrPawnBase* NearestPawnToCursor = PC->GetNearestPawnToCursor();
	return NearestPawnToCursor;
}
