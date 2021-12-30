// Copyright 2015 Cameron Angus. All Rights Reserved.

#include "UtilitySelectionMethods/BTUtilitySelectionMethod_ScoreWeight.h"
#include "BTComposite_Utility.h"
#include "BTUtilitySelectionMethod_Proportional.h"


struct FIndexedUtilityValue
{
	int32 ChildIdx;
	int32 Value;
	int32 Weight;

	inline bool operator< (FIndexedUtilityValue const& Other) const
	{
		return Value > Other.Value;
	}
};

template < typename ActiveFilter >
static int32 WeightOrderingSelect(TArray< FIndexedUtilityValue > const& Values, ActiveFilter const& Filter, int32 const Sum)
{
	int32 SelectionValue = FMath::RandRange(0, Sum);
	int32 Cumulative = 0;
	for (int32 Idx = 0; Idx < Values.Num(); ++Idx)
	{
		if (Filter(Idx))
		{
			Cumulative += Values[Idx].Weight;
			if (SelectionValue <= Cumulative)
			{
				return Idx;
			}
		}
	}

	check(false);
	return 0;
}

static void WeightOrdering(TArray< FIndexedUtilityValue > const& UtilityValues, TArray< int32 > & ExecutionOrdering)
{
	auto const Count = UtilityValues.Num();

	// Calculate the sum of all utility values
	int32 UtilitySum = 0;
	for (auto Ut : UtilityValues)
	{
		UtilitySum += Ut.Weight;
	}

	// Create an array denoting which children remain to be chosen
	TArray< bool > Remaining;
	Remaining.Init(true, Count);

	ExecutionOrdering.Reset(Count);
	for (int32 Iteration = 0; Iteration < Count - 1; ++Iteration)
	{
		// Select the next highest priority child, based on the utility values of those remaining
		auto ChildIdx = WeightOrderingSelect(UtilityValues, [&Remaining](int32 Idx) { return Remaining[Idx]; }, UtilitySum);

		// Reduce the remaining utility sum and remove the child from the remaining array
		UtilitySum -= UtilityValues[ChildIdx].Weight;
		UtilitySum = FMath::Max(UtilitySum, 0);
		Remaining[ChildIdx] = false;

		ExecutionOrdering.Add(UtilityValues[ChildIdx].ChildIdx);
	}

	// Fill in the only remaining child index
	ExecutionOrdering.Add(UtilityValues[Remaining.IndexOfByKey(true)].ChildIdx);
}


namespace UtilitySelection
{
	void ScoreWeightOrdering(TArray< FBTUtilityScoreWeight > const& UtilityValues, FUtilityExecutionOrdering& ExecutionOrdering)
	{
		
		// Index the values and sort them
		auto const Count = UtilityValues.Num();
		TArray< FIndexedUtilityValue > IndexedValues;
		IndexedValues.SetNumUninitialized(Count);
		for (int32 ChildIdx = 0; ChildIdx < Count; ++ChildIdx)
		{
			IndexedValues[ChildIdx].ChildIdx = ChildIdx;
			IndexedValues[ChildIdx].Value = UtilityValues[ChildIdx]._score;
			IndexedValues[ChildIdx].Weight = UtilityValues[ChildIdx]._weight;
		}

		IndexedValues.Sort();

		// Output the sorted indexes
		//ExecutionOrdering.SetNumUninitialized(Count);
		ExecutionOrdering.Reset(0);

		TArray< FIndexedUtilityValue > tmpArr;
		TArray<int32> tmpOrderArr;

		for (int32 i = 0; i < Count;i++)
		{
			FIndexedUtilityValue var = IndexedValues[i];
			tmpArr.Add(var);
			
			if ( i == Count-1 || var.Value != IndexedValues[i+1].Value  )
			{
				if (tmpArr.Num() == 1)
				{
					//ExecutionOrdering[i] = var.ChildIdx;
					ExecutionOrdering.Add(var.ChildIdx);
				}
				else if (tmpArr.Num() > 1)
				{
					WeightOrdering(tmpArr,tmpOrderArr);
					ExecutionOrdering.Append(tmpOrderArr);
				}
				tmpArr.Reset(0);
				
			}

		}

	}
}



