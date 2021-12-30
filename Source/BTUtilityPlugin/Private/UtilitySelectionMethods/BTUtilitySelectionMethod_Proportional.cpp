// Copyright 2015 Cameron Angus. All Rights Reserved.

#include "UtilitySelectionMethods/BTUtilitySelectionMethod_Proportional.h"
#include "BTComposite_Utility.h"


template < typename ActiveFilter >
static int32 ProportionalSelect(TArray< FBTUtilityScoreWeight > const& Values, ActiveFilter const& Filter, int32 const Sum)
{
	int32 SelectionValue = FMath::RandRange(0, Sum);
	int32 Cumulative = 0;
	for (int32 Idx = 0; Idx < Values.Num(); ++Idx)
	{
		if (Filter(Idx))
		{
			Cumulative += Values[Idx]._weight;
			if (SelectionValue <= Cumulative)
			{
				return Idx;
			}
		}
	}

	check(false);
	return 0;
}

template < typename ActiveFilter >
static int32 ProportionalSelect(TArray< FBTUtilityScoreWeight > const& Values, ActiveFilter const& Filter)
{
	int32 Sum = 0;
	for (int32 Idx = 0; Idx < Values.Num(); ++Idx)
	{
		if (Filter(Idx))
		{
			Sum += Values[Idx]._weight;
		}
	}

	return ProportionalSelect(Values, Filter, Sum);
}


namespace UtilitySelection
{
	void ProportionalOrdering(TArray< FBTUtilityScoreWeight > const& UtilityValues, FUtilityExecutionOrdering& ExecutionOrdering)
	{
		auto const Count = UtilityValues.Num();

		// Calculate the sum of all utility values
		int32 UtilitySum = 0;
		for (auto Ut : UtilityValues)
		{
			UtilitySum += Ut._weight;
		}

		// Create an array denoting which children remain to be chosen
		TArray< bool > Remaining;
		Remaining.Init(true, Count);

		ExecutionOrdering.Reset(Count);
		for (int32 Iteration = 0; Iteration < Count - 1; ++Iteration)
		{
			// Select the next highest priority child, based on the utility values of those remaining
			auto ChildIdx = ProportionalSelect(UtilityValues, [&Remaining](int32 Idx){ return Remaining[Idx]; }, UtilitySum);
			ExecutionOrdering.Add(ChildIdx);

			// Reduce the remaining utility sum and remove the child from the remaining array
			UtilitySum -= UtilityValues[ChildIdx]._weight;
			UtilitySum = FMath::Max(UtilitySum, 0);
			Remaining[ChildIdx] = false;
		}

		// Fill in the only remaining child index
		ExecutionOrdering.Add(Remaining.IndexOfByKey(true));
	}
}


#if 0

int32 UBTUtilitySelectionMethod_Proportional::SelectOption(TArray< float > const& UtilityValues)
{
	return ProportionalSelect(UtilityValues, [](int32){ return true; });
}

void UBTUtilitySelectionMethod_Proportional::GenerateOrdering(TArray< float > const& UtilityValues, FUtilityExecutionOrdering& ExecutionOrdering)
{
	auto const Count = UtilityValues.Num();

	// Calculate the sum of all utility values
	float UtilitySum = 0.0f;
	for (auto Ut : UtilityValues)
	{
		UtilitySum += Ut;
	}

	// Create an array denoting which children remain to be chosen
	TArray< bool > Remaining;
	Remaining.Init(true, Count);

	ExecutionOrdering.Reset(Count);
	for (int32 Iteration = 0; Iteration < Count - 1; ++Iteration)
	{
		// Select the next highest priority child, based on the utility values of those remaining
		auto ChildIdx = ProportionalSelect(UtilityValues, [&Remaining](int32 Idx){ return Remaining[Idx]; }, UtilitySum);
		ExecutionOrdering.Add(ChildIdx);

		// Reduce the remaining utility sum and remove the child from the remaining array
		UtilitySum -= UtilityValues[ChildIdx];
		UtilitySum = FMath::Max(UtilitySum, 0.0f);
		Remaining[ChildIdx] = false;
	}

	// Fill in the only remaining child index
	ExecutionOrdering.Add(Remaining.IndexOfByKey(true));
}

FString UBTUtilitySelectionMethod_Proportional::GetDesc()
{
	return FString::Printf(TEXT("Proportional"));
}

#endif

