// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/LrPatrolRouteComponent.h"


FVector ULrPatrolRouteComponent::GetNextPatrolPoint()
{
	if (PatrolPoints.IsEmpty())
	{
		return FVector::ZeroVector;
	}

	AActor* Point = PatrolPoints[CurrentIndex];

	CurrentIndex =(CurrentIndex + 1) % PatrolPoints.Num();

	return Point->GetActorLocation();
}
