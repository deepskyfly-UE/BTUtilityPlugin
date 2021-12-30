// Copyright 2015 Cameron Angus. All Rights Reserved.

#include "Composites/BTComposite_Utility.h"
#include "Decorators/BTDecorator_UtilityFunction.h"
#include "UtilitySelectionMethods/BTUtilitySelectionMethod_Highest.h"
#include "UtilitySelectionMethods/BTUtilitySelectionMethod_Proportional.h"
#include "BTUtilitySelectionMethod_ScoreWeight.h"

UBTComposite_Utility::UBTComposite_Utility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Utility";
	bUseNodeActivationNotify = true;

	SelectionMethod = EUtilitySelectionMethod::ScoreWeight;

	//OnNextChild.BindUObject(this, &UBTComposite_Utility::GetNextChildHandler);
}

void UBTComposite_Utility::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	FBTUtilityMemory* UtilityMemory = reinterpret_cast<FBTUtilityMemory*>(NodeMemory);
	if (InitType == EBTMemoryInit::Initialize)
	{
		// Invoke the constructor for our TArray
		new (&UtilityMemory->ExecutionOrdering) FUtilityExecutionOrdering();
	}
}

FString UBTComposite_Utility::GetStaticDescription() const
{
	switch (SelectionMethod)
	{
	case EUtilitySelectionMethod::ScoreOnly:
		return TEXT("ScoreOnly selection");

	case EUtilitySelectionMethod::WeightOnly:
		return TEXT("WeightOnly selection");

	case EUtilitySelectionMethod::ScoreWeight:
		return TEXT("ScoreWeight selection");

	default:
		check(false);
		return{};
	}
}

void UBTComposite_Utility::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	Super::DescribeRuntimeValues(OwnerComp,NodeMemory,Verbosity,Values);
#if WITH_EDITOR
	if(!OwnerComp.IsRunning())
		return;

	FBTUtilityMemory* mem = reinterpret_cast<FBTUtilityMemory*>(NodeMemory);
	for (int i=0;i<mem->ExecutionOrdering.Num();i++)
	{
		int id = mem->ExecutionOrdering[i];
		if(id >=0 && id <  mem->UtilityValues.Num())
		{
			FBTUtilityScoreWeight& value = mem->UtilityValues[id];
			if (GetChildNode(id))
			{
				Values.Add(FString::Printf(TEXT("%s [%d, %d]"), *GetChildNode(id)->GetNodeName(), value._score,value._weight));
			}
			else
			{
				Values.Add(FString::Printf(TEXT("%s [%d, %d]"), TEXT("None"), value._score, value._weight));
			}
		}

	}
#endif
}

// const UBTDecorator_UtilityFunction* UBTComposite_Utility::FindChildUtilityFunction(int32 ChildIndex) const
// {
// 	auto const& ChildInfo = Children[ChildIndex];
// 	for (auto Dec : ChildInfo.Decorators)
// 	{
// 		auto AsUtilFunc = Cast< UBTDecorator_UtilityFunction >(Dec);
// 		if (AsUtilFunc)
// 		{
// 			// Take the first one. Multiple utility function decorators on a single node is a user
// 			// error, and generates a warning in the behavior tree editor.
// 			return AsUtilFunc;
// 		}
// 	}
// 
// 	// Child does not have a utility function decorator
// 	return nullptr;
// }

FBTUtilityScoreWeight UBTComposite_Utility::GetChildUtilityScore(FBehaviorTreeSearchData& SearchData,int32 ChildIndex) const
{
	FBTUtilityScoreWeight retScore;

	auto const& ChildInfo = Children[ChildIndex];
	for (auto Dec : ChildInfo.Decorators)
	{
		auto AsUtilFunc = Cast< UBTDecorator_UtilityFunction >(Dec);
		if (AsUtilFunc)
		{
			retScore.Add(AsUtilFunc->WrappedCalculateUtility(SearchData.OwnerComp,GetNodeMemory< uint8 >(SearchData)));
		}
	}

	return retScore;
}

#if 0
bool UBTComposite_Utility::ShouldConsiderChild(UBehaviorTreeComponent& OwnerComp, int32 InstanceIdx, int32 ChildIdx) const
{
	auto UtilityFunc = Children[ChildIdx].UtilityFunction;

	switch (UtilityFunc->DecoratorPolicy)
	{
	case EUtilityChildConsiderationPolicy::IgnoreIfCantRun:
		/*
		@TODO: Is there any performance or correctness issue here?
		We are doing a decorator check, which will then be repeated for the selected child node (and really
		should give the same result)
		*/
		return DoDecoratorsAllowExecution(OwnerComp, InstanceIdx, ChildIdx);

	default:
		return true;
	}
}
#endif

bool UBTComposite_Utility::EvaluateUtilityScores(FBehaviorTreeSearchData& SearchData, TArray< FBTUtilityScoreWeight >& OutScores) const
{
	bool bIsNonZeroScore = false;
	// Loop through utility children
	for(int32 Idx = 0; Idx < GetChildrenNum(); ++Idx)
	{
		// Calculate utility value
		auto Score = GetChildUtilityScore(SearchData,Idx);

		OutScores.Add(Score);
		bIsNonZeroScore = bIsNonZeroScore || !Score.IsEmpty();
	}

	return bIsNonZeroScore;
}

void UBTComposite_Utility::NotifyNodeActivation(FBehaviorTreeSearchData& SearchData) const
{
	FBTUtilityMemory* NodeMemory = GetNodeMemory<FBTUtilityMemory>(SearchData);

	// Evaluate utility scores for each child
	TArray< FBTUtilityScoreWeight > UtilityValues;
	bool bNonZeroUtility = EvaluateUtilityScores(SearchData, UtilityValues);

#if WITH_EDITOR
	NodeMemory->UtilityValues = UtilityValues;
#endif

	// Generate ordering
	switch (SelectionMethod)
	{
	case EUtilitySelectionMethod::ScoreOnly:
		UtilitySelection::PriorityOrdering(UtilityValues, NodeMemory->ExecutionOrdering);
		break;
	case EUtilitySelectionMethod::WeightOnly:
		UtilitySelection::ProportionalOrdering(UtilityValues, NodeMemory->ExecutionOrdering);
		break;
	case EUtilitySelectionMethod::ScoreWeight:
		UtilitySelection::ScoreWeightOrdering(UtilityValues, NodeMemory->ExecutionOrdering);
		break;
	default:
		check(false);
	}
}

int32 UBTComposite_Utility::GetNextChildHandler(FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const
{
	FBTUtilityMemory* NodeMemory = GetNodeMemory<FBTUtilityMemory>(SearchData);

	// success = quit
	int32 NextChildIdx = BTSpecialChild::ReturnToParent;

	check(NodeMemory->ExecutionOrdering.Num() == GetChildrenNum());
	if (PrevChild == BTSpecialChild::NotInitialized)
	{
		// newly activated: start from first in the ordering
		NextChildIdx = NodeMemory->ExecutionOrdering[0];
	}
	else if (LastResult == EBTNodeResult::Failed)
	{
		// @NOTE: Linear search to find position in ordering of last executed child. This could be avoided but overhead is negligible
		// so seems better to avoid storing extra state in the node memory.
		int32 OrderingIndex = NodeMemory->ExecutionOrdering.IndexOfByKey(PrevChild) + 1;
		if (OrderingIndex < NodeMemory->ExecutionOrdering.Num())
		{
			// failed = choose next child in the ordering
			NextChildIdx = NodeMemory->ExecutionOrdering[OrderingIndex];
		}
	}

	return NextChildIdx;
}

uint16 UBTComposite_Utility::GetInstanceMemorySize() const
{
	return sizeof(FBTUtilityMemory);
}

#if WITH_EDITOR

FName UBTComposite_Utility::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Composite.Utility.Icon");
}

#endif


