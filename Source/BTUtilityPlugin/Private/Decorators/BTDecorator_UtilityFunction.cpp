// Copyright 2015 Cameron Angus. All Rights Reserved.

#include "Decorators/BTDecorator_UtilityFunction.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BTComposite_Utility.h"


UBTDecorator_UtilityFunction::UBTDecorator_UtilityFunction(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "UnknownUtilityFunction";

	bAllowAbortNone = false;
	bAllowAbortLowerPri = false;
	bAllowAbortChildNodes = false;
}

FBTUtilityScoreWeight UBTDecorator_UtilityFunction::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return FBTUtilityScoreWeight();
}

FBTUtilityScoreWeight UBTDecorator_UtilityFunction::WrappedCalculateUtility(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBTDecorator_UtilityFunction* NodeOb = bCreateNodeInstance ? (const UBTDecorator_UtilityFunction*)GetNodeInstance(OwnerComp, NodeMemory) : this;
	return NodeOb ? NodeOb->CalculateUtilityValue(OwnerComp, NodeMemory) : FBTUtilityScoreWeight();
}



#if 0
float UBTDecorator_UtilityFunction::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return 0.0f;
}

float UBTDecorator_UtilityFunction::WrappedCalculateUtility(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBTDecorator_UtilityFunction* NodeOb = bCreateNodeInstance ? (const UBTDecorator_UtilityFunction*)GetNodeInstance(OwnerComp, NodeMemory) : this;
	return NodeOb ? NodeOb->CalculateUtilityValue(OwnerComp, NodeMemory) : 0.0f;
}

#endif

