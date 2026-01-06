UCLASS()
class UDashStartTransition : public UBaseMovementModeTransition
{
	GENERATED_BODY()

public:
	virtual FTransitionEvalResult Evaluate_Implementation(
		const FSimulationTickParams& Params) const override
	{
		const FMoverExampleAbilityInputs* Inputs = Params.StartState.InputCmd.InputCollection.FindDataByType<FMoverExampleAbilityInputs>();

		if (Inputs && Inputs->bIsDashJustPressed)
		{
			return FTransitionEvalResult::MakeImmediate(ExtendedModeNames::Dash);
		}

		return FTransitionEvalResult::NoTransition();
	}
};
