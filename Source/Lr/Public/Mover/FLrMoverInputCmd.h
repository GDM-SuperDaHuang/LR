#pragma once
#include "CoreMinimal.h"
#include "MoverDataModelTypes.h"
#include "FLrMoverInputCmd.generated.h"


USTRUCT(BlueprintType)
struct FLrMoverInputCmd : public FCharacterDefaultInputs
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = Mover)
	bool bIsCrouchPressed = false;//蹲伏

	UPROPERTY(BlueprintReadWrite, Category = Mover)
	bool bIsBlinkPressed = false;//冲刺

	// UPROPERTY(BlueprintReadWrite, Category = Mover)
	// bool bIsJumpPressed = false;//跳跃
	
	virtual FMoverDataStructBase* Clone() const override
	{
		return new FLrMoverInputCmd(*this);
	}

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override
	{
		Super::NetSerialize(Ar, Map, bOutSuccess);
		// Сериализуем флаги (битовая маска для экономии трафика)
		uint8 Flags = (bIsCrouchPressed << 0) | (bIsBlinkPressed << 1);
		Ar.SerializeBits(&Flags, 2);

		if (Ar.IsLoading())
		{
			bIsCrouchPressed = (Flags & (1 << 0)) != 0;
			bIsBlinkPressed = (Flags & (1 << 1)) != 0;
			// bIsJumpPressed   = (Flags & (1 << 2)) != 0;

		}

		return true;
	}

	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
};
