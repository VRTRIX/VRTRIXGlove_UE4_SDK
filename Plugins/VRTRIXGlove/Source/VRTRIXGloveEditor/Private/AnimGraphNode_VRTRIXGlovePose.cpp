// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimGraphNode_VRTRIXGlovePose.h"

#define LOCTEXT_NAMESPACE "VRTRIXGloveAnimNode"

UAnimGraphNode_VRTRIXGlovePose::UAnimGraphNode_VRTRIXGlovePose(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FText UAnimGraphNode_VRTRIXGlovePose::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "VRTRIXGlove Pose");
}

FText UAnimGraphNode_VRTRIXGlovePose::GetTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Retrieves the current pose associated with the supplied subject");
}

FText UAnimGraphNode_VRTRIXGlovePose::GetMenuCategory() const
{
	return LOCTEXT("NodeCategory", "VRTRIXGlove");
}

#undef LOCTEXT_NAMESPACE