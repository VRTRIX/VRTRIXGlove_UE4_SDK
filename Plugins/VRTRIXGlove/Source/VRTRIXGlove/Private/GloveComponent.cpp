// Fill out your copyright notice in the Description page of Project Settings.

#include "GloveComponent.h"

#include "Engine/Texture2D.h"
#include "Engine/Engine.h"
#include "IXRTrackingSystem.h"
#include "IHeadMountedDisplay.h"

#if WITH_EDITOR
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#endif

DEFINE_LOG_CATEGORY(LogVRTRIXGlovePlugin);

static FORCEINLINE FMatrix ToFMatrix(const vr::HmdMatrix34_t& tm)
{
	// Rows and columns are swapped between vr::HmdMatrix34_t and FMatrix
	return FMatrix(
		FPlane(tm.m[0][0], tm.m[1][0], tm.m[2][0], 0.0f),
		FPlane(tm.m[0][1], tm.m[1][1], tm.m[2][1], 0.0f),
		FPlane(tm.m[0][2], tm.m[1][2], tm.m[2][2], 0.0f),
		FPlane(tm.m[0][3], tm.m[1][3], tm.m[2][3], 1.0f));
}

void UGloveComponent::CreateBoneIndexToBoneNameMap(FHandBonesName names)
{
	if (names.IndexInHand != "None" && names.MiddleInHand != "None" && names.RingInHand != "None" && names.PinkyInHand != "None") {
		BoneIndexToBoneNameMap.Emplace(0, names.Wrist);
		BoneIndexToBoneNameMap.Emplace(1, names.Thumb1);
		BoneIndexToBoneNameMap.Emplace(2, names.Thumb2);
		BoneIndexToBoneNameMap.Emplace(3, names.Thumb3);
		BoneIndexToBoneNameMap.Emplace(4, names.IndexInHand);
		BoneIndexToBoneNameMap.Emplace(5, names.Index1);
		BoneIndexToBoneNameMap.Emplace(6, names.Index2);
		BoneIndexToBoneNameMap.Emplace(7, names.Index3);
		BoneIndexToBoneNameMap.Emplace(8, names.MiddleInHand);
		BoneIndexToBoneNameMap.Emplace(9, names.Middle1);
		BoneIndexToBoneNameMap.Emplace(10, names.Middle2);
		BoneIndexToBoneNameMap.Emplace(11, names.Middle3);
		BoneIndexToBoneNameMap.Emplace(12, names.RingInHand);
		BoneIndexToBoneNameMap.Emplace(13, names.Ring1);
		BoneIndexToBoneNameMap.Emplace(14, names.Ring2);
		BoneIndexToBoneNameMap.Emplace(15, names.Ring3);
		BoneIndexToBoneNameMap.Emplace(16, names.PinkyInHand);
		BoneIndexToBoneNameMap.Emplace(17, names.Pinky1);
		BoneIndexToBoneNameMap.Emplace(18, names.Pinky2);
		BoneIndexToBoneNameMap.Emplace(19, names.Pinky3);
	}
	else {
		BoneIndexToBoneNameMap.Emplace(0, names.Wrist);
		BoneIndexToBoneNameMap.Emplace(1, names.Thumb1);
		BoneIndexToBoneNameMap.Emplace(2, names.Thumb2);
		BoneIndexToBoneNameMap.Emplace(3, names.Thumb3);
		BoneIndexToBoneNameMap.Emplace(4, names.Index1);
		BoneIndexToBoneNameMap.Emplace(5, names.Index2);
		BoneIndexToBoneNameMap.Emplace(6, names.Index3);
		BoneIndexToBoneNameMap.Emplace(7, names.Middle1);
		BoneIndexToBoneNameMap.Emplace(8, names.Middle2);
		BoneIndexToBoneNameMap.Emplace(9, names.Middle3);
		BoneIndexToBoneNameMap.Emplace(10, names.Ring1);
		BoneIndexToBoneNameMap.Emplace(11, names.Ring2);
		BoneIndexToBoneNameMap.Emplace(12, names.Ring3);
		BoneIndexToBoneNameMap.Emplace(13, names.Pinky1);
		BoneIndexToBoneNameMap.Emplace(14, names.Pinky2);
		BoneIndexToBoneNameMap.Emplace(15, names.Pinky3);
	}

}

double UGloveComponent::CalculateBendAngle(const VRTRIX::VRTRIXQuaternion_t & q1, const VRTRIX::VRTRIXQuaternion_t & q2)
{
	FQuat q1_fquat(q1.qx, q1.qy, q1.qz, q1.qw);
	FQuat q2_fquat(q2.qx, q2.qy, q2.qz, q2.qw);
	FQuat offset = q1_fquat.Inverse() * q2_fquat;
	return FMath::RadiansToDegrees(atan2(2.0f * offset.W * offset.Z + 2.0f * offset.X * offset.Y, 1 - 2.0f * (offset.Z * offset.Z + offset.X * offset.X)));
}

double UGloveComponent::GetFingerBendAngle(VRTRIX::Joint finger)
{
	return CalculateBendAngle(m_pose.imuData[VRTRIX::Wrist_Joint], m_pose.imuData[finger]);
}

void UGloveComponent::PerformAlgorithmTuning()
{
	VRTRIX::EIMUError error = VRTRIX::IMUError_None;
	VRTRIX::VRTRIXVector_t offset;

	if (m_ThumbOffset[0] != ThumbOffset[0]) {
		offset = { ThumbOffset[0].X, ThumbOffset[0].Y, ThumbOffset[0].Z };
		pDataGlove->AlgorithmTuning(error, VRTRIX::Thumb_Proximal, VRTRIX::AlgorithmConfig_ThumbOffset, 0, offset);
		m_ThumbOffset[0] = ThumbOffset[0];
	}

	if (m_ThumbOffset[1] != ThumbOffset[1]) {
		offset = { ThumbOffset[1].X, ThumbOffset[1].Y, ThumbOffset[1].Z };
		pDataGlove->AlgorithmTuning(error, VRTRIX::Thumb_Intermediate, VRTRIX::AlgorithmConfig_ThumbOffset, 0, offset);
		m_ThumbOffset[1] = ThumbOffset[1];
	}

	if (m_ThumbOffset[2] != ThumbOffset[2]) {
		offset = { ThumbOffset[2].X, ThumbOffset[2].Y, ThumbOffset[2].Z };
		pDataGlove->AlgorithmTuning(error, VRTRIX::Thumb_Distal, VRTRIX::AlgorithmConfig_ThumbOffset, 0, offset);
		m_ThumbOffset[2] = ThumbOffset[2];
	}

	if (m_ThumbProximalSlerp != ThumbProximalSlerp) {
		pDataGlove->AlgorithmTuning(error, VRTRIX::Thumb_Proximal, VRTRIX::AlgorithmConfig_ProximalSlerpDown, ThumbProximalSlerp);
		m_ThumbProximalSlerp = ThumbProximalSlerp;
	}

	if (m_ThumbMiddleSlerp != ThumbMiddleSlerp) {
		pDataGlove->AlgorithmTuning(error, VRTRIX::Thumb_Distal, VRTRIX::AlgorithmConfig_DistalSlerpDown, ThumbMiddleSlerp);
		m_ThumbMiddleSlerp = ThumbMiddleSlerp;
	}

	if (m_FingerSpacing != FingerSpacing) {
		pDataGlove->AlgorithmTuning(error, VRTRIX::Wrist_Joint, VRTRIX::AlgorithmConfig_FingerSpcaing, FingerSpacing);
		m_FingerSpacing = FingerSpacing;
	}

	if (m_FinalFingerSpacing != FinalFingerSpacing) {
		pDataGlove->AlgorithmTuning(error, VRTRIX::Wrist_Joint, VRTRIX::AlgorithmConfig_FinalFingerSpacing, FinalFingerSpacing);
		m_FinalFingerSpacing = FinalFingerSpacing;
	}
}

// Sets default values for this component's properties
UGloveComponent::UGloveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	rotation.Init(FRotator(0, 0, 0), VRTRIX::Joint_MAX);
	FingerBendingAngle.Init(0.0, 5);
	alignmentPose = FQuat::Identity;
	// ...
}


// Called when the game starts
void UGloveComponent::BeginPlay()
{
	Super::BeginPlay();
	LWristTrackerPitchOffset = FQuat::Identity;
	RWristTrackerPitchOffset = FQuat::Identity;
	ml_axisoffset = FMatrix::Identity;
	mr_axisoffset = FMatrix::Identity;
	m_LHTrackerIndex = 64;
	m_RHTrackerIndex = 64;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (HandType == Hand::Left) {
				ml_axisoffset.M[j][i] = AxisOffset[i][j];
			}
			else {
				mr_axisoffset.M[j][i] = AxisOffset[i][j];
			}
		}
	}
	event_gesture_num = Gesture_Event.Num();
	state_gesture_num = Gesture_NonEvent.Num();
	//创建骨骼映射；
	CreateBoneIndexToBoneNameMap(handBoneNames);
	// ...
	if (bIsVREnabled && GetTrackingSystem()) {
		GetTrackerIndex();
	}
	OnConnectGloves();
}

void UGloveComponent::EndPlay(const EEndPlayReason::Type EEndPlayReason)
{
	OnDisconnectGloves();
}

void UGloveComponent::OnReceiveNewPose(VRTRIX::Pose pose)
{
	PerformAlgorithmTuning();
	m_pose = pose;
	FingerBendingAngle[0] = GetFingerBendAngle(VRTRIX::Thumb_Intermediate);
	FingerBendingAngle[1] = GetFingerBendAngle(VRTRIX::Index_Intermediate);
	FingerBendingAngle[2] = GetFingerBendAngle(VRTRIX::Middle_Intermediate);
	FingerBendingAngle[3] = GetFingerBendAngle(VRTRIX::Ring_Intermediate);
	FingerBendingAngle[4] = GetFingerBendAngle(VRTRIX::Pinky_Intermediate);

	for (int i = 0; i < VRTRIX::Joint_MAX; ++i) {
		FQuat quat = { pose.imuData[i].qx, pose.imuData[i].qy,pose.imuData[i].qz,pose.imuData[i].qw };

		FVector offset_vec = (pose.type == VRTRIX::Hand_Left) ?
			ml_axisoffset.TransformVector(FVector(quat.X, quat.Y, quat.Z)) :
			mr_axisoffset.TransformVector(FVector(quat.X, quat.Y, quat.Z));
		quat = { offset_vec[0], offset_vec[1], offset_vec[2], quat.W };

		if (!bIsVREnabled) {
			if (pose.type == VRTRIX::Hand_Left && !bIsLOffsetCal && i == (int)VRTRIX::Wrist_Joint && quat != FQuat::Identity) {
				initialPoseOffset = InitialPoseOffset.Quaternion() * quat.Inverse();
				bIsLOffsetCal = true;
			}
			else if (pose.type == VRTRIX::Hand_Right && !bIsROffsetCal && i == (int)VRTRIX::Wrist_Joint && quat != FQuat::Identity) {
				initialPoseOffset = InitialPoseOffset.Quaternion() * quat.Inverse();
				bIsROffsetCal = true;
			}

			if (bIsWristAlignEnabled && i == (int)VRTRIX::Wrist_Joint && quat != FQuat::Identity) {
				initialPoseOffset = alignmentPose * quat.Inverse();
			}
		}
		else {
			if (pose.type == VRTRIX::Hand_Left  && i == (int)VRTRIX::Wrist_Joint) {
				FRotator tracker_rot;
				FVector tracker_loc;
				if (USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(m_LHTrackerIndex, tracker_loc, tracker_rot)) {
					if (!bIsLOffsetCal) {
						LWristTrackerPitchOffset = FQuat(FVector::ForwardVector, FMath::DegreesToRadians(tracker_rot.Roll + 90.0f));
						UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] Left Hand Glove connected to channel: %d"), pose.channel);
						bIsLOffsetCal = true;
					}
					FQuat target = tracker_rot.Quaternion() * LWristTrackerPitchOffset * WristTrackerRotOffset.Quaternion();
					initialPoseOffset = target * quat.Inverse();
				}
			}
			else if (pose.type == VRTRIX::Hand_Right  && i == (int)VRTRIX::Wrist_Joint) {
				FRotator tracker_rot;
				FVector tracker_loc;
				if (USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(m_RHTrackerIndex, tracker_loc, tracker_rot)) {
					if (!bIsROffsetCal) {
						RWristTrackerPitchOffset = FQuat(FVector::ForwardVector, FMath::DegreesToRadians(tracker_rot.Roll - 90.0f));
						UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] Right Hand Glove connected to channel: %d"), pose.channel);
						bIsROffsetCal = true;
					}
					FQuat target = tracker_rot.Quaternion() * RWristTrackerPitchOffset * WristTrackerRotOffset.Quaternion();
					initialPoseOffset = target * quat.Inverse();
				}
			}
		}

		rotation[i] = (i == (int)VRTRIX::Wrist_Joint) ?
			(initialPoseOffset * quat).Rotator() :
			(initialPoseOffset * quat * WristFingerOffset.Quaternion()).Rotator();
	}

	if (ShowDebugInfo) {
		UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("Bending Angle (Thumb to Pinky): %f, %f, %f, %f, %f"), FingerBendingAngle[0], FingerBendingAngle[1], FingerBendingAngle[2], FingerBendingAngle[3], FingerBendingAngle[4]);
	}

}

void UGloveComponent::OnConnectGloves()
{
	VRTRIX::EInitError eInitError = VRTRIX::InitError_None;
	VRTRIX::EIMUError eIMUError = VRTRIX::IMUError_None;
	//Initialize event handler.
	VRTRIX::IVRTRIXIMUEventHandler* pEventHandler = new CVRTRIXIMUEventHandler();
	//Initialize data glove.
	if (HardwareVersion == HardwareVersion::DK1 || HardwareVersion == HardwareVersion::DK2) {
		pDataGlove = AdvancedMode ? InitDataGlove(eInitError, VRTRIX::InitMode_Advanced, VRTRIX::DK2) :
			InitDataGlove(eInitError, VRTRIX::InitMode_Normal, VRTRIX::DK2);
	}
	else if (HardwareVersion == HardwareVersion::PRO) {
		pDataGlove = AdvancedMode ? InitDataGlove(eInitError, VRTRIX::InitMode_Advanced, VRTRIX::PRO) :
			InitDataGlove(eInitError, VRTRIX::InitMode_Normal, VRTRIX::PRO);
	}
	else if (HardwareVersion == HardwareVersion::PRO11) {
		pDataGlove = AdvancedMode ? InitDataGlove(eInitError, VRTRIX::InitMode_Advanced, VRTRIX::PRO11) :
			InitDataGlove(eInitError, VRTRIX::InitMode_Normal, VRTRIX::PRO11);
	}
	else {
		UE_LOG(LogVRTRIXGlovePlugin, Error, TEXT("[GLOVES PULGIN] Invalid data glove hardware version!"));
		return;
	}

	if (eInitError != VRTRIX::InitError_None) {
		UE_LOG(LogVRTRIXGlovePlugin, Error, TEXT("[GLOVES PULGIN] Unable to init Glove runtime"));
		return;
	}
	if (HandType == Hand::Right) {
		type = VRTRIX::Hand_Right;
	}
	else if (HandType == Hand::Left) {
		type = VRTRIX::Hand_Left;
	}

	//Register event call back and perform events handling/pose updating.
	pDataGlove->RegisterIMUDataCallback(pEventHandler, this);

	//Connect Data Gloves
	VRTRIX::PortInfo portInfo;
	portInfo.IP = std::string(TCHAR_TO_UTF8(*ServerIP));
	portInfo.port = std::to_string(11002 + (int)GloveID);
	portInfo.type = type;
	pDataGlove->ConnectDataGlove(eIMUError, portInfo);

	if (eIMUError == VRTRIX::IMUError_None) {
		//Print out full port information
		UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] Server IP: %s"), *FString(portInfo.IP.c_str()));
		UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] Port: %s"), *FString(portInfo.port.c_str()));
		bIsDataGlovePortOpened = true;
	}
	else {
		if (type == VRTRIX::Hand_Left) {
			UE_LOG(LogVRTRIXGlovePlugin, Error, TEXT("[GLOVES PULGIN] Unable to init Left Hand Glove: %d"), (int)eIMUError);
		}
		else {
			UE_LOG(LogVRTRIXGlovePlugin, Error, TEXT("[GLOVES PULGIN] Unable to init Right Hand Glove: %d"), (int)eIMUError);
		}
		bIsDataGlovePortOpened = false;
	}
}

void UGloveComponent::OnDisconnectGloves()
{
	VRTRIX::EIMUError eIMUError;
	if (bIsDataGlovePortOpened) {
		pDataGlove->DisconnectDataGlove(eIMUError);
		bIsDataGlovePortOpened = false;
	}
}

bool UGloveComponent::GetTrackingSystem()
{
	if (!GEngine->XRSystem.IsValid() || (GEngine->XRSystem->GetSystemName() != SteamVRSystemName))
	{
		UE_LOG(LogVRTRIXGlovePlugin, Error, TEXT("[GLOVES PULGIN] Unable to get tracking system."));
		return false;
	}

	vr::HmdError HmdErr;
	VRSystem = (vr::IVRSystem*)vr::VR_GetGenericInterface(vr::IVRSystem_Version, &HmdErr);
	VRCompositor = (vr::IVRCompositor*)vr::VR_GetGenericInterface(vr::IVRCompositor_Version, &HmdErr);
	if (VRSystem == NULL) UE_LOG(LogVRTRIXGlovePlugin, Error, TEXT("[GLOVES PULGIN] Unable to get tracking system."));
	return (VRSystem != NULL);
}

void UGloveComponent::OnReconnect()
{
	UE_LOG(LogVRTRIXGlovePlugin, Warning, TEXT("[GLOVES PULGIN] Try to reconnect data gloves."));
	OnDisconnectGloves();
	OnConnectGloves();
}

void UGloveComponent::OnTriggerHaptics(int duration)
{
	if (!bIsDataGloveConnected) return;

	FString handTypeString = (type == VRTRIX::Hand_Left) ? "Left Hand Glove" : "Right Hand Glove";
	UE_LOG(LogVRTRIXGlovePlugin, Warning, TEXT("[GLOVES PULGIN] %s trigger haptics, duration: %d."), *handTypeString, duration);

	VRTRIX::EIMUError eIMUError;
	pDataGlove->VibratePeriod(eIMUError, duration);
}

void UGloveComponent::OnToggleHaptics()
{
	if (!bIsDataGloveConnected) return;

	FString handTypeString = (type == VRTRIX::Hand_Left) ? "Left Hand Glove" : "Right Hand Glove";
	UE_LOG(LogVRTRIXGlovePlugin, Warning, TEXT("[GLOVES PULGIN] %s trigger haptics."), *handTypeString);

	VRTRIX::EIMUError eIMUError;
	pDataGlove->ToggleVibration(eIMUError);
}

void UGloveComponent::OrientationAlignment()
{
	if (!bIsDataGloveConnected) return;
	if (type == VRTRIX::Hand_Left) {
		bIsLOffsetCal = false;
	}
	else {
		bIsROffsetCal = false;
	}
	VRTRIX::EIMUError error;
	pDataGlove->SoftwareAlign(error);
}

void UGloveComponent::ApplyHandMoCapWorldSpaceRotation(UPoseableMeshComponent *SkinMesh, FRotator alignment)
{
	if (!bIsDataGloveConnected) return;

	SetWristAlignment(alignment);

	TArray<FRotator> FingerWorldSpaceRotation;
	FingerWorldSpaceRotation.Init(FRotator::ZeroRotator, BoneIndexToBoneNameMap.Num());

	if (BoneIndexToBoneNameMap.Num() == VRTRIX::Joint_MAX) {
		for (int i = 0; i < BoneIndexToBoneNameMap.Num(); i++) {
			FingerWorldSpaceRotation[i] = rotation[i];
		}
	}
	else if (BoneIndexToBoneNameMap.Num() == VRTRIX::Joint_MAX + 4) {
		for (int i = 0; i < BoneIndexToBoneNameMap.Num(); i++) {
			int QuatIndex = 0;
			if (i == 4 || i == 8 || i == 12 || i == 16) QuatIndex = 0;
			else if (i < 4) QuatIndex = i;
			else if (i < 8) QuatIndex = i - 1;
			else if (i < 12) QuatIndex = i - 2;
			else if (i < 16) QuatIndex = i - 3;
			else QuatIndex = i - 4;
			FingerWorldSpaceRotation[i] = rotation[QuatIndex];
		}
	}
	else return;

	for (int i = 0; i < BoneIndexToBoneNameMap.Num(); i++)
	{
		SkinMesh->SetBoneRotationByName(BoneIndexToBoneNameMap[i], FingerWorldSpaceRotation[i], EBoneSpaces::WorldSpace);
	}
}

void UGloveComponent::GetTrackerIndex()
{
	if (!VRSystem) {
		return;
	}
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		//if (VRSystem->GetTrackedDeviceClass(nDevice) == vr::TrackedDeviceClass_GenericTracker) {
		if (VRSystem->IsTrackedDeviceConnected(nDevice)) {
			FString renderModel;
			EBPOVRResultSwitch result;
			GetVRDevicePropertyString(EVRDeviceProperty_String::Prop_RenderModelName_String_1003, nDevice, renderModel, result);
			//UE_LOG(LogVRTRIXGlovePlugin, Error, TEXT("[GLOVES PULGIN] renderModel: %s"), *renderModel);
			if (result == EBPOVRResultSwitch::OnSucceeded) {
				if (renderModel == "LH") m_LHTrackerIndex = nDevice;
				if (renderModel == "RH") m_RHTrackerIndex = nDevice;
			}
		}
		//}
	}
	UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] LHIndex: %d"), m_LHTrackerIndex);
	UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] RHIndex: %d"), m_RHTrackerIndex);
}

FTransform UGloveComponent::ApplyTrackerOffset()
{
	FRotator tracker_rot;
	FVector tracker_loc;

	switch (type) {
	case(VRTRIX::Hand_Left): {
		if (!USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(m_LHTrackerIndex, tracker_loc, tracker_rot)) {
			return FTransform::Identity;
		}
		break;
	}
	case(VRTRIX::Hand_Right): {
		if (!USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(m_RHTrackerIndex, tracker_loc, tracker_rot)) {
			return FTransform::Identity;
		}
		break;
	}
	}
	FVector new_positon = tracker_loc + tracker_rot.Quaternion() * WristTrackerOffset;
	return FTransform(tracker_rot, new_positon, FVector(1, 1, 1));
}

FTransform UGloveComponent::GetTrackerTransform() {
	FRotator tracker_rot;
	FVector tracker_loc;

	switch (type) {
	case(VRTRIX::Hand_Left): {
		if (!USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(m_LHTrackerIndex, tracker_loc, tracker_rot)) {
			return FTransform::Identity;
		}
		break;
	}
	case(VRTRIX::Hand_Right): {
		if (!USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(m_RHTrackerIndex, tracker_loc, tracker_rot)) {
			return FTransform::Identity;
		}
		break;
	}
	}
	return FTransform(tracker_rot, tracker_loc, FVector(1, 1, 1));
}

void UGloveComponent::SetWristAlignment(FRotator alignment)
{
	alignmentPose = alignment.Quaternion();
}

void UGloveComponent::Calculate_Gesture_Event()
{
	int gesture_index = 0;
	// Gesture 0
	if (gesture_index < event_gesture_num)
	{
		if (Gesture_Event[gesture_index].Gesture_State)
		{
			if (!Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
			{
				Gesture_Released_0.Broadcast();
			}
		}
		else if (Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
		{
			Gesture_Triggered_0.Broadcast();
		}
		gesture_index++;
	}
	else { return; }
	// Gesture 1
	if (gesture_index < event_gesture_num)
	{
		if (Gesture_Event[gesture_index].Gesture_State)
		{
			if (!Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
			{
				Gesture_Released_1.Broadcast();
			}
		}
		else if (Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
		{
			Gesture_Triggered_1.Broadcast();
		}
		gesture_index++;
	}
	else { return; }
	// Gesture 2
	if (gesture_index < event_gesture_num)
	{
		if (Gesture_Event[gesture_index].Gesture_State)
		{
			if (!Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
			{
				Gesture_Released_2.Broadcast();
			}

		}
		else if (Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
		{
			Gesture_Triggered_2.Broadcast();
		}
		gesture_index++;
	}
	else { return; }
	// Gesture 3
	if (gesture_index < event_gesture_num)
	{
		if (Gesture_Event[gesture_index].Gesture_State)
		{
			if (!Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
			{
				Gesture_Released_3.Broadcast();
			}

		}
		else if (Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
		{
			Gesture_Triggered_3.Broadcast();
		}
		gesture_index++;
	}
	else { return; }
	// Gesture 4
	if (gesture_index < event_gesture_num)
	{
		if (Gesture_Event[gesture_index].Gesture_State)
		{
			if (!Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
			{
				Gesture_Released_4.Broadcast();
			}

		}
		else if (Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
		{
			Gesture_Triggered_4.Broadcast();
		}
		gesture_index++;
	}
	else { return; }
	// Gesture 5
	if (gesture_index < event_gesture_num)
	{
		if (Gesture_Event[gesture_index].Gesture_State)
		{
			if (!Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
			{
				Gesture_Released_5.Broadcast();
			}

		}
		else if (Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
		{
			Gesture_Triggered_5.Broadcast();
		}
		gesture_index++;
	}
	else { return; }
	// Gesture 6
	if (gesture_index < event_gesture_num)
	{
		if (Gesture_Event[gesture_index].Gesture_State)
		{
			if (!Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
			{
				Gesture_Released_6.Broadcast();
			}

		}
		else if (Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
		{
			Gesture_Triggered_6.Broadcast();
		}
		gesture_index++;
	}
	else { return; }
	// Gesture 7
	if (gesture_index < event_gesture_num)
	{
		if (Gesture_Event[gesture_index].Gesture_State)
		{
			if (!Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
			{
				Gesture_Released_7.Broadcast();
			}
		}
		else if (Gesture_Event[gesture_index].TriggerPositionCheck(FingerBendingAngle))
		{
			Gesture_Triggered_7.Broadcast();
		}
		gesture_index++;
	}
	else { return; }
}

void UGloveComponent::Calculate_Gesture_State()
{
	for (int gesture_index = 0; gesture_index < state_gesture_num; gesture_index++)
	{
		Gesture_NonEvent[gesture_index].TriggerPositionCheck(FingerBendingAngle);
	}
}

// Called every frame
void UGloveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
	UGloveComponent::Calculate_Gesture_Event();
	UGloveComponent::Calculate_Gesture_State();
}

static vr::ETrackedDeviceProperty VREnumToString(const FString& enumName, uint8 value)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *enumName, true);

	if (!EnumPtr)
		return vr::ETrackedDeviceProperty::Prop_Invalid;

	FString EnumName = EnumPtr->GetNameStringByIndex(value).Right(4);

	if (EnumName.IsEmpty() || EnumName.Len() < 4)
		return vr::ETrackedDeviceProperty::Prop_Invalid;

	return static_cast<vr::ETrackedDeviceProperty>(FCString::Atoi(*EnumName));
}

void UGloveComponent::GetVRDevicePropertyString(EVRDeviceProperty_String PropertyToRetrieve, int32 DeviceID, FString & StringValue, EBPOVRResultSwitch & Result)
{

#if !STEAMVR_SUPPORTED_PLATFORM
	Result = EBPOVRResultSwitch::OnFailed;
	return;
#else
	vr::TrackedPropertyError pError = vr::TrackedPropertyError::TrackedProp_Success;
	vr::ETrackedDeviceProperty EnumPropertyValue = VREnumToString(TEXT("EVRDeviceProperty_String"), static_cast<uint8>(PropertyToRetrieve));
	if (EnumPropertyValue == vr::ETrackedDeviceProperty::Prop_Invalid)
	{
		Result = EBPOVRResultSwitch::OnFailed;
		return;
	}
	char charvalue[vr::k_unMaxPropertyStringSize];
	uint32_t buffersize = 255;
	uint32_t ret = VRSystem->GetStringTrackedDeviceProperty(DeviceID, EnumPropertyValue, charvalue, buffersize, &pError);

	if (pError != vr::TrackedPropertyError::TrackedProp_Success)
	{
		Result = EBPOVRResultSwitch::OnFailed;
		return;
	}
	StringValue = FString(ANSI_TO_TCHAR(charvalue));
	Result = EBPOVRResultSwitch::OnSucceeded;
	return;
#endif
}

void UGloveComponent::GetVRDevicePropertyBool(EVRDeviceProperty_Bool PropertyToRetrieve, int32 DeviceID, bool & BoolValue, EBPOVRResultSwitch & Result)
{
#if !STEAMVR_SUPPORTED_PLATFORM
	Result = EBPOVRResultSwitch::OnFailed;
	return;
#else
	vr::TrackedPropertyError pError = vr::TrackedPropertyError::TrackedProp_Success;
	vr::ETrackedDeviceProperty EnumPropertyValue = VREnumToString(TEXT("EVRDeviceProperty_Bool"), static_cast<uint8>(PropertyToRetrieve));
	if (EnumPropertyValue == vr::ETrackedDeviceProperty::Prop_Invalid)
	{
		Result = EBPOVRResultSwitch::OnFailed;
		return;
	}
	bool ret = VRSystem->GetBoolTrackedDeviceProperty(DeviceID, EnumPropertyValue, &pError);
	if (pError != vr::TrackedPropertyError::TrackedProp_Success)
	{
		Result = EBPOVRResultSwitch::OnFailed;
		return;
	}
	BoolValue = ret;
	Result = EBPOVRResultSwitch::OnSucceeded;
	return;

#endif
}

void UGloveComponent::GetVRDevicePropertyFloat(EVRDeviceProperty_Float PropertyToRetrieve, int32 DeviceID, float & FloatValue, EBPOVRResultSwitch & Result)
{
#if !STEAMVR_SUPPORTED_PLATFORM
	Result = EBPOVRResultSwitch::OnFailed;
	return;
#else
	vr::TrackedPropertyError pError = vr::TrackedPropertyError::TrackedProp_Success;
	vr::ETrackedDeviceProperty EnumPropertyValue = VREnumToString(TEXT("EVRDeviceProperty_Float"), static_cast<uint8>(PropertyToRetrieve));
	if (EnumPropertyValue == vr::ETrackedDeviceProperty::Prop_Invalid)
	{
		Result = EBPOVRResultSwitch::OnFailed;
		return;
	}
	float ret = VRSystem->GetFloatTrackedDeviceProperty(DeviceID, EnumPropertyValue, &pError);
	if (pError != vr::TrackedPropertyError::TrackedProp_Success)
	{
		Result = EBPOVRResultSwitch::OnFailed;
		return;
	}
	FloatValue = ret;
	Result = EBPOVRResultSwitch::OnSucceeded;
	return;

#endif
}

void UGloveComponent::GetVRDevicePropertyInt32(EVRDeviceProperty_Int32 PropertyToRetrieve, int32 DeviceID, int32 & IntValue, EBPOVRResultSwitch & Result)
{
#if !STEAMVR_SUPPORTED_PLATFORM
	Result = EBPOVRResultSwitch::OnFailed;
	return;
#else
	vr::TrackedPropertyError pError = vr::TrackedPropertyError::TrackedProp_Success;
	vr::ETrackedDeviceProperty EnumPropertyValue = VREnumToString(TEXT("EVRDeviceProperty_Int32"), static_cast<uint8>(PropertyToRetrieve));
	if (EnumPropertyValue == vr::ETrackedDeviceProperty::Prop_Invalid)
	{
		Result = EBPOVRResultSwitch::OnFailed;
		return;
	}
	int32 ret = VRSystem->GetInt32TrackedDeviceProperty(DeviceID, EnumPropertyValue, &pError);
	if (pError != vr::TrackedPropertyError::TrackedProp_Success)
	{
		Result = EBPOVRResultSwitch::OnFailed;
		return;
	}
	IntValue = ret;
	Result = EBPOVRResultSwitch::OnSucceeded;
	return;

#endif
}

void UGloveComponent::GetVRDevicePropertyUInt64(EVRDeviceProperty_UInt64 PropertyToRetrieve, int32 DeviceID, FString & UInt64Value, EBPOVRResultSwitch & Result)
{
#if !STEAMVR_SUPPORTED_PLATFORM
	Result = EBPOVRResultSwitch::OnFailed;
	return;
#else
	vr::TrackedPropertyError pError = vr::TrackedPropertyError::TrackedProp_Success;
	vr::ETrackedDeviceProperty EnumPropertyValue = VREnumToString(TEXT("EVRDeviceProperty_UInt64"), static_cast<uint8>(PropertyToRetrieve));
	if (EnumPropertyValue == vr::ETrackedDeviceProperty::Prop_Invalid)
	{
		Result = EBPOVRResultSwitch::OnFailed;
		return;
	}
	uint64 ret = VRSystem->GetUint64TrackedDeviceProperty(DeviceID, EnumPropertyValue, &pError);
	if (pError != vr::TrackedPropertyError::TrackedProp_Success)
	{
		Result = EBPOVRResultSwitch::OnFailed;
		return;
	}
	UInt64Value = FString::Printf(TEXT("%llu"), ret);
	Result = EBPOVRResultSwitch::OnSucceeded;
	return;

#endif
}

bool FMyGesture::TriggerPositionCheck(TArray<float> Current_Position)
{
	int upper_num = this->Upper_bound.Num() - 1;
	for (; upper_num >= 0; upper_num--)
	{
		//if any finger's angle go up cross upper bound, set Position State indicator to fasle,return false for check function
		if ((Current_Position[upper_num] > this->Upper_bound[upper_num]) && (this->Upper_bound[upper_num] != 0))
		{
			this->Gesture_State = false;
			return false;
		}
	}

	int lower_num = this->Lower_bound.Num() - 1;
	for (; lower_num >= 0; lower_num--)
	{
		//if any finger's angle go up cross upper bound, set Position State indicator to fasle,return false for check function
		if ((Current_Position[lower_num] < this->Lower_bound[lower_num]) && (this->Lower_bound[lower_num] != 0))
		{
			this->Gesture_State = false;
			return false;
		}
	}
	// Position check for all fingers pass, set Position State indicator to true,return true for check function
	this->Gesture_State = true;
	return true;
}
