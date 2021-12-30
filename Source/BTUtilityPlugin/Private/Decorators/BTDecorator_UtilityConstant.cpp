// Copyright 2015 Cameron Angus. All Rights Reserved.

#include "Decorators/BTDecorator_UtilityConstant.h"


UBTDecorator_UtilityConstant::UBTDecorator_UtilityConstant(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Constant Utility";
	//_scoreWeight = FBTUtilityScoreWeight::ZeroScoreWeight;
}

FBTUtilityScoreWeight UBTDecorator_UtilityConstant::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return _scoreWeight;
}

FString UBTDecorator_UtilityConstant::GetStaticDescription() const
{
	return FString::Printf(TEXT("Score,Weight: [ %d, %d]"), _scoreWeight._score, _scoreWeight._weight);
}

void UBTDecorator_UtilityConstant::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	Super::DescribeRuntimeValues(OwnerComp, NodeMemory, Verbosity, Values);
}

