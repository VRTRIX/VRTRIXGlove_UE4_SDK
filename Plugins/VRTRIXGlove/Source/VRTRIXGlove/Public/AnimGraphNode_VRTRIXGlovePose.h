// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GloveComponent.h"
#include "AnimGraphNode_Base.h"
#include "AnimNode_VRTRIXGlovePose.h"
#include "AnimGraphNode_VRTRIXGlovePose.generated.h"

/**
 * 
 */
UCLASS()
class VRTRIXGLOVE_API UAnimGraphNode_VRTRIXGlovePose : public UAnimGraphNode_Base
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_VRTRIXGlovePose Node;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const;
};
