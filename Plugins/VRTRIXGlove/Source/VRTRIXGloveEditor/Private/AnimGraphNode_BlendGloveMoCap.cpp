// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimGraphNode_BlendGloveMoCap.h"

#define LOCTEXT_NAMESPACE "VRTRIXGloveAnimNode"

UAnimGraphNode_BlendGloveMoCap::UAnimGraphNode_BlendGloveMoCap(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FText UAnimGraphNode_BlendGloveMoCap::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "Blend Glove Data In Mocap");
}

FText UAnimGraphNode_BlendGloveMoCap::GetTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Add glove data with mocap full body data");
}

FText UAnimGraphNode_BlendGloveMoCap::GetMenuCategory() const
{
	return LOCTEXT("NodeCategory", "VRTRIXGlove Blend Mocap Data");
}

FLinearColor UAnimGraphNode_BlendGloveMoCap::GetNodeTitleColor() const
{
	return FLinearColor(0.75f, 0.75f, 0.75f);
}

#undef LOCTEXT_NAMESPACE