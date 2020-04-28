// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNode_BlendGloveMoCap.h"
#include "Animation/AnimInstanceProxy.h"

FAnimNode_BlendGloveMoCap::FAnimNode_BlendGloveMoCap()
{
}

void FAnimNode_BlendGloveMoCap::Initialize_AnyThread(const FAnimationInitializeContext & Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);

	Glove.Initialize(Context);
	MoCap.Initialize(Context);

	TArray<UActorComponent*> Comps = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetAnimInstance()->GetOwningActor()->GetComponentsByClass(UGloveComponent::StaticClass());
	for (auto& component : Comps)
	{
		UGloveComponent* glove = dynamic_cast<UGloveComponent*> (component);
		if (glove->HandType == Hand::Left) LHGloveComponent = glove;
		else if (glove->HandType == Hand::Right) RHGloveComponent = glove;
		//UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] Find Glove Component %d."), (int)glove->HandType);
	}
}

void FAnimNode_BlendGloveMoCap::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
{
	Glove.CacheBones(Context);
	MoCap.CacheBones(Context);
}

void FAnimNode_BlendGloveMoCap::Update_AnyThread(const FAnimationUpdateContext & Context)
{
	Glove.Update(Context);
	MoCap.Update(Context);
	EvaluateGraphExposedInputs.Execute(Context);
}

void FAnimNode_BlendGloveMoCap::Evaluate_AnyThread(FPoseContext & Output)
{
	MoCap.Evaluate(Output);

	if (LHGloveComponent == nullptr || RHGloveComponent == nullptr) return;
	if (!LHGloveComponent->bIsDataGloveConnected || !RHGloveComponent->bIsDataGloveConnected) return;

	FCSPose<FCompactPose> FinalPose;
	FinalPose.InitPose(&Output.Pose.GetBoneContainer());
	const FBoneContainer Container = FinalPose.GetPose().GetBoneContainer();
	
	FPoseContext GloveContext(Output);
	Glove.Evaluate(GloveContext);

	int startIndex = bIsAlignWrist ? 1 : 0;
	TArray<int32> handBoneIndex;
	for (int i = startIndex; i < LHGloveComponent->BoneIndexToBoneNameMap.Num(); i++)
	{
		int32 MeshBoneIndex = Container.GetPoseBoneIndexForBoneName(LHGloveComponent->BoneIndexToBoneNameMap[i]);
		if (MeshBoneIndex != INDEX_NONE)
		{
			handBoneIndex.Push(MeshBoneIndex);
		}
	}

	for (int i = startIndex; i < RHGloveComponent->BoneIndexToBoneNameMap.Num(); i++)
	{
		int32 MeshBoneIndex = Container.GetPoseBoneIndexForBoneName(RHGloveComponent->BoneIndexToBoneNameMap[i]);
		if (MeshBoneIndex != INDEX_NONE)
		{
			handBoneIndex.Push(MeshBoneIndex);
		}
	}

	//UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] Total Glove Bone: %d"), handBoneIndex.Num());

	for (FCompactPoseBoneIndex BoneIndex : Output.Pose.ForEachBoneIndex())
	{
		if (handBoneIndex.Find(BoneIndex.GetInt()) != INDEX_NONE) {
			Output.Pose[BoneIndex] = GloveContext.Pose[BoneIndex];
		}
	}
}
