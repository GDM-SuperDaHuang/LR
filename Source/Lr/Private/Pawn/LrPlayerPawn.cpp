// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/LrPlayerPawn.h"

#include "MoverComponent.h"
#include "Mover/LrBaseMovementMode.h"

ALrPlayerPawn::ALrPlayerPawn()
{
	
}

void ALrPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	if (MoverComponent)
	{
		// 注册到Network Prediction驱动自动同步和预测
		MoverComponent->RegisterNetworkPrediction();

		// 注册运动模式
		MoverComponent->AddMovementModeFromClass(TEXT("Base"), UBaseMovementMode::StaticClass());
		MoverComponent->AddMovementModeFromClass(TEXT("Jump"), UJumpMovementMode::StaticClass());
		MoverComponent->AddMovementModeFromClass(TEXT("Dash"), UDashMovementMode::StaticClass());

		MoverComponent->QueueNextMode(TEXT("Base"));

		// 绑定输入生产委托
		MoverComponent->BindProduceInputDelegate(
			UMoverComponent::FProduceInputDelegate::CreateUObject(this, &AMyMoverPawn::ProduceMoverInput)
		);
	}
	// RootComponent = MoverComponent;// 设置为根组件（处理物理/位置）
}

void ALrPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ALrPlayerPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CharacterMotionComponent = FindComponentByClass<UCharacterMoverComponent>();

	if (CharacterMotionComponent)
	{
		if (USceneComponent* UpdatedComponent = CharacterMotionComponent->GetUpdatedComponent())
		{
			UpdatedComponent->SetCanEverAffectNavigation(bCanAffectNavigationGeneration);
		}
	}
}


void ALrPlayerPawn::ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult)
{
	OnProduceInput((float)SimTimeMs, InputCmdResult);
}


void ALrPlayerPawn::OnProduceInput(float DeltaMs, FMoverInputCmdContext& OutInputCmd)
{
	/**
	 * 第一次 FindOrAddMutableDataByType
	 * 构造一个 FMoverDataStructBase 派生实例
	 * 生命周期：仅属于这一帧的 InputCmd
	 */
	FDashState& CharacterInputs = OutInputCmd.InputCollection.FindOrAddMutableDataByType<FDashState>();

	if (GetController() == nullptr)
	{
		if (GetLocalRole() == ENetRole::ROLE_Authority && GetRemoteRole() == ENetRole::ROLE_SimulatedProxy)
		{
			static const FCharacterDefaultInputs DoNothingInput;
			// If we get here, that means this pawn is not currently possessed and we're choosing to provide default do-nothing input
			CharacterInputs = DoNothingInput;
		}

		// We don't have a local controller so we can't run the code below. This is ok. Simulated proxies will just use previous input when extrapolating
		return;
	}

	if (USpringArmComponent* SpringComp = FindComponentByClass<USpringArmComponent>())
	{
		// This is not best practice: do not search for component every frame
		SpringComp->bUsePawnControlRotation = true;
	}

	CharacterInputs.ControlRotation = FRotator::ZeroRotator;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		CharacterInputs.ControlRotation = PC->GetControlRotation();
	}

	bool bRequestedNavMovement = false;
	if (NavMoverComponent)
	{
		bRequestedNavMovement = NavMoverComponent->ConsumeNavMovementData(CachedMoveInputIntent, CachedMoveInputVelocity);
	}
	
	// Favor velocity input 
	bool bUsingInputIntentForMove = CachedMoveInputVelocity.IsZero();

	if (bUsingInputIntentForMove)
	{
		const FVector FinalDirectionalIntent = CharacterInputs.ControlRotation.RotateVector(CachedMoveInputIntent);
		CharacterInputs.SetMoveInput(EMoveInputType::DirectionalIntent, FinalDirectionalIntent);
	}
	else
	{
		CharacterInputs.SetMoveInput(EMoveInputType::Velocity, CachedMoveInputVelocity);
	}

	// Normally cached input is cleared by OnMoveCompleted input event but that won't be called if movement came from nav movement
	if (bRequestedNavMovement)
	{
		CachedMoveInputIntent = FVector::ZeroVector;
		CachedMoveInputVelocity = FVector::ZeroVector;
	}
	
	static float RotationMagMin(1e-3);

	const bool bHasAffirmativeMoveInput = (CharacterInputs.GetMoveInput().Size() >= RotationMagMin);
	
	// Figure out intended orientation
	CharacterInputs.OrientationIntent = FVector::ZeroVector;


	if (bHasAffirmativeMoveInput)
	{
		if (bOrientRotationToMovement)
		{
			// set the intent to the actors movement direction
			CharacterInputs.OrientationIntent = CharacterInputs.GetMoveInput().GetSafeNormal();
		}
		else
		{
			// set intent to the the control rotation - often a player's camera rotation
			CharacterInputs.OrientationIntent = CharacterInputs.ControlRotation.Vector().GetSafeNormal();
		}

		LastAffirmativeMoveInput = CharacterInputs.GetMoveInput();
	}
	else if (bMaintainLastInputOrientation)
	{
		// There is no movement intent, so use the last-known affirmative move input
		CharacterInputs.OrientationIntent = LastAffirmativeMoveInput;
	}
	
	CharacterInputs.bIsJumpPressed = bIsJumpPressed;
	CharacterInputs.bIsJumpJustPressed = bIsJumpJustPressed;

	if (bShouldToggleFlying)
	{
		if (!bIsFlyingActive)
		{
			CharacterInputs.SuggestedMovementMode = DefaultModeNames::Flying;
		}
		else
		{
			CharacterInputs.SuggestedMovementMode = DefaultModeNames::Falling;
		}

		bIsFlyingActive = !bIsFlyingActive;
	}
	else
	{
		CharacterInputs.SuggestedMovementMode = NAME_None;
	}

	// Convert inputs to be relative to the current movement base (depending on options and state)
	CharacterInputs.bUsingMovementBase = false;

	if (bUseBaseRelativeMovement)
	{
		if (const UCharacterMoverComponent* MoverComp = GetComponentByClass<UCharacterMoverComponent>())
		{
			if (UPrimitiveComponent* MovementBase = MoverComp->GetMovementBase())
			{
				FName MovementBaseBoneName = MoverComp->GetMovementBaseBoneName();

				FVector RelativeMoveInput, RelativeOrientDir;

				UBasedMovementUtils::TransformWorldDirectionToBased(MovementBase, MovementBaseBoneName, CharacterInputs.GetMoveInput(), RelativeMoveInput);
				UBasedMovementUtils::TransformWorldDirectionToBased(MovementBase, MovementBaseBoneName, CharacterInputs.OrientationIntent, RelativeOrientDir);

				CharacterInputs.SetMoveInput(CharacterInputs.GetMoveInputType(), RelativeMoveInput);
				CharacterInputs.OrientationIntent = RelativeOrientDir;

				CharacterInputs.bUsingMovementBase = true;
				CharacterInputs.MovementBase = MovementBase;
				CharacterInputs.MovementBaseBoneName = MovementBaseBoneName;
			}
		}
	}

	// Clear/consume temporal movement inputs. We are not consuming others in the event that the game world is ticking at a lower rate than the Mover simulation. 
	// In that case, we want most input to carry over between simulation frames.
	{

		bIsJumpJustPressed = false;
		bShouldToggleFlying = false;
	}
}
