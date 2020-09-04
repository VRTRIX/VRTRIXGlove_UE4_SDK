// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNode_VRTRIXGlovePose.h"
#include "Animation/AnimInstanceProxy.h"

FAnimNode_VRTRIXGlovePose::FAnimNode_VRTRIXGlovePose()
{
}

void FAnimNode_VRTRIXGlovePose::Initialize_AnyThread(const FAnimationInitializeContext & Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);

	TArray<UActorComponent*> Comps = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetAnimInstance()->GetOwningActor()->GetComponentsByClass(UGloveComponent::StaticClass());
	for (auto& component : Comps)
	{
		UGloveComponent* glove = dynamic_cast<UGloveComponent*> (component);
		if (glove->HandType == Hand::Left) LHGloveComponent = glove;
		else if(glove->HandType == Hand::Right) RHGloveComponent = glove;
		//UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] Find Glove Component %d."), (int)glove->HandType);
	}
}

void FAnimNode_VRTRIXGlovePose::Update_AnyThread(const FAnimationUpdateContext & Context)
{
	EvaluateGraphExposedInputs.Execute(Context);
	if (LHGloveComponent == nullptr || RHGloveComponent == nullptr) {
		TArray<UActorComponent*> Comps = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetAnimInstance()->GetOwningActor()->GetComponentsByClass(UGloveComponent::StaticClass());
		for (auto& component : Comps)
		{
			UGloveComponent* glove = dynamic_cast<UGloveComponent*> (component);
			if (glove->HandType == Hand::Left) LHGloveComponent = glove;
			else if (glove->HandType == Hand::Right) RHGloveComponent = glove;
			//UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] Find Glove Component %d."), (int)glove->HandType);
		}
	}
}

void FAnimNode_VRTRIXGlovePose::Evaluate_AnyThread(FPoseContext & Output)
{
	//Output.ResetToRefPose();
	if (LHGloveComponent == nullptr || RHGloveComponent == nullptr) return;
	if (!LHGloveComponent->bIsDataGloveConnected || !RHGloveComponent->bIsDataGloveConnected) return;


	FCSPose<FCompactPose> pose;
	pose.InitPose(&Output.Pose.GetBoneContainer());
	const FBoneContainer Container = pose.GetPose().GetBoneContainer();

	TArray<FQuat> LHFingerRotation, RHFingerRotation;
	LHFingerRotation.Init(FQuat::Identity, LHGloveComponent->BoneIndexToBoneNameMap.Num());
	RHFingerRotation.Init(FQuat::Identity, RHGloveComponent->BoneIndexToBoneNameMap.Num());

	if (LHGloveComponent->BoneIndexToBoneNameMap.Num() == VRTRIX::Joint_MAX && RHGloveComponent->BoneIndexToBoneNameMap.Num() == VRTRIX::Joint_MAX) {
		for (int i = 0; i < LHGloveComponent->BoneIndexToBoneNameMap.Num(); i++) {
			LHFingerRotation[i] = LHGloveComponent->rotation[i].Quaternion();
			RHFingerRotation[i] = RHGloveComponent->rotation[i].Quaternion();
		}
	}
	else if (LHGloveComponent->BoneIndexToBoneNameMap.Num() == VRTRIX::Joint_MAX + 4 && RHGloveComponent->BoneIndexToBoneNameMap.Num() == VRTRIX::Joint_MAX + 4) {
		for (int i = 0; i < LHGloveComponent->BoneIndexToBoneNameMap.Num(); i++) {
			int QuatIndex = 0;
			if (i == 4 || i == 8 || i == 12 || i == 16) QuatIndex = 0;
			else if (i < 4) QuatIndex = i;
			else if (i < 8) QuatIndex = i - 1;
			else if (i < 12) QuatIndex = i - 2;
			else if (i < 16) QuatIndex = i - 3;
			else QuatIndex = i - 4;
			LHFingerRotation[i] = LHGloveComponent->rotation[QuatIndex].Quaternion();
			RHFingerRotation[i] = RHGloveComponent->rotation[QuatIndex].Quaternion();
		}
	}
	else return;

	for (int i = 0; i < LHGloveComponent->BoneIndexToBoneNameMap.Num(); i++)
	{
		int32 MeshBoneIndex = Container.GetPoseBoneIndexForBoneName(LHGloveComponent->BoneIndexToBoneNameMap[i]);
		if (MeshBoneIndex != INDEX_NONE)
		{
			FCompactPoseBoneIndex CPIndex = Container.MakeCompactPoseIndex(FMeshPoseBoneIndex(MeshBoneIndex));
			FTransform CSTransformFromSource = pose.GetComponentSpaceTransform(CPIndex);
			CSTransformFromSource.SetRotation(LHFingerRotation[i]);
			pose.SetComponentSpaceTransform(CPIndex, CSTransformFromSource);
		}
	}

	for (int i = 0; i < RHGloveComponent->BoneIndexToBoneNameMap.Num(); i++)
	{
		int32 MeshBoneIndex = Container.GetPoseBoneIndexForBoneName(RHGloveComponent->BoneIndexToBoneNameMap[i]);
		if (MeshBoneIndex != INDEX_NONE)
		{
			FCompactPoseBoneIndex CPIndex = Container.MakeCompactPoseIndex(FMeshPoseBoneIndex(MeshBoneIndex));
			FTransform CSTransformFromSource = pose.GetComponentSpaceTransform(CPIndex);
			CSTransformFromSource.SetRotation(RHFingerRotation[i]);
			pose.SetComponentSpaceTransform(CPIndex, CSTransformFromSource);
		}
	}

	pose.ConvertToLocalPoses(Output.Pose);
}
