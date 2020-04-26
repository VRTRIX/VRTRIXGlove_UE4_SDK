// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include "GloveComponent.h"
class UGloveComponent;
#include "AnimNode_VRTRIXGlovePose.generated.h"
/**
 * 
 */
USTRUCT(BlueprintInternalUseOnly)
struct VRTRIXGLOVE_API FAnimNode_VRTRIXGlovePose : public FAnimNode_Base
{
	GENERATED_USTRUCT_BODY()

	FAnimNode_VRTRIXGlovePose();

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;

	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext & Context) override {}

	virtual void Update_AnyThread(const FAnimationUpdateContext & Context) override;

	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	// End of FAnimNode_Base interface

private:
	UGloveComponent* LHGloveComponent;
	UGloveComponent* RHGloveComponent;
};
