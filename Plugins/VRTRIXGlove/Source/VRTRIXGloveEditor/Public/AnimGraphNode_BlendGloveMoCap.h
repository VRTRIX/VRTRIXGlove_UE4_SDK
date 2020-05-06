// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "AnimNode_BlendGloveMoCap.h"
#include "AnimGraphNode_BlendGloveMoCap.generated.h"

/**
 * 
 */
UCLASS()
class UAnimGraphNode_BlendGloveMoCap : public UAnimGraphNode_Base
{
	GENERATED_UCLASS_BODY()
	
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_BlendGloveMoCap Node;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetMenuCategory() const;
};
