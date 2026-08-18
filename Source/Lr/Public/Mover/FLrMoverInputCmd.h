#pragma once
#include "CoreMinimal.h"
#include "MoverDataModelTypes.h"
#include "FLrMoverInputCmd.generated.h"


USTRUCT(BlueprintType)
struct FLrMoverInputCmd : public FCharacterDefaultInputs
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = Mover)
	float FlyVertical;

	UPROPERTY(BlueprintReadWrite, Category = Mover)
	bool bIsCrouchPressed = false; //蹲伏

	UPROPERTY(BlueprintReadWrite, Category = Mover)
	bool bIsBlinkPressed = false; //冲刺

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


USTRUCT()
struct FLrFlyAuxState : public FMoverDataStructBase
{
	GENERATED_BODY()

	// 当前平滑后的 Roll
	UPROPERTY()
	float Roll = 0.f;

	// 当前平滑后的 Pitch
	UPROPERTY()
	float Pitch = 0.f;

	/** 激活 Fly 时记录的目标高度 */
	UPROPERTY()
	float TakeOffTargetZ = 0.f;

	/** 是否还在起飞阶段 */
	UPROPERTY()
	bool bTakingOff = false;

	// 必须告诉 Mover：我的真实 ScriptStruct 是谁
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FLrFlyAuxState::StaticStruct();
	}

	// Mover 需要能够复制这个 DataStruct
	virtual FMoverDataStructBase* Clone() const override
	{
		return new FLrFlyAuxState(*this);
	}

	// 网络预测/服务器校正时比较状态
	virtual bool ShouldReconcile(
		const FMoverDataStructBase& AuthorityState) const override
	{
		const FLrFlyAuxState* Other =
			static_cast<const FLrFlyAuxState*>(&AuthorityState);

		if (!Other)
		{
			return true;
		}

		constexpr float Tolerance = 0.01f;

		return
			!FMath::IsNearlyEqual(Roll, Other->Roll, Tolerance) ||
			!FMath::IsNearlyEqual(Pitch, Other->Pitch, Tolerance);
	}

	// 用于插值
	virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct) override
	{
		const FLrFlyAuxState* FromState = static_cast<const FLrFlyAuxState*>(&From);
		const FLrFlyAuxState* ToState = static_cast<const FLrFlyAuxState*>(&To);
		Roll = FMath::Lerp(FromState->Roll, ToState->Roll, Pct);
		Pitch = FMath::Lerp(FromState->Pitch, ToState->Pitch, Pct);
	}

	virtual void ToString(FAnsiStringBuilderBase& Out) const override
	{
		Out.Appendf("FlyAux Roll=%.3f Pitch=%.3f", Roll, Pitch);
	}
};
