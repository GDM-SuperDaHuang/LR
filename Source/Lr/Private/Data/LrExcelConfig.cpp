// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/LrExcelConfig.h"

#include "GameplayTagContainer.h"

const TArray<FLrInitASRow*> ULrExcelConfig::FindAllAS() const
{
	TArray<FLrInitASRow*> AllRows;
	if (!LrASDT) return AllRows;
	
	// 2. 定义一个上下文 context 字符串
	// 这个字符串纯粹是为了“报错提示”用的。如果表结构对不上或者读取出错，
	// UE 会在 Log 里输出：Warning: ... FString::Printf(TEXT("%s: 报错原因"), *ContextString)
	FString ContextString(TEXT("Batch Loading AS"));
	LrASDT->GetAllRows<FLrInitASRow>(ContextString, AllRows);
	return AllRows;
}

const TArray<FLrProjectileConfigRow*> ULrExcelConfig::FindAllProjectileConfig() const
{
	TArray<FLrProjectileConfigRow*> AllRows;
	if (!LrASDT) return AllRows;
	
	// 2. 定义一个上下文 context 字符串
	// 这个字符串纯粹是为了“报错提示”用的。如果表结构对不上或者读取出错，
	// UE 会在 Log 里输出：Warning: ... FString::Printf(TEXT("%s: 报错原因"), *ContextString)
	FString ContextString(TEXT("Batch Loading ProjectileConfig"));
	LrASDT->GetAllRows<FLrProjectileConfigRow>(ContextString, AllRows);
	return AllRows;
}
