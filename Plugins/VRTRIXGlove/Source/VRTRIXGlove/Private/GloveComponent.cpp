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

IMotionController* GetSteamMotionController()
{
	static FName DeviceTypeName(TEXT("SteamVRController"));
	TArray<IMotionController*> MotionControllers = IModularFeatures::Get().GetModularFeatureImplementations<IMotionController>(IMotionController::GetModularFeatureName());
	for (IMotionController* MotionController : MotionControllers)
	{
		if (MotionController->GetMotionControllerDeviceTypeName() == DeviceTypeName)
		{
			return MotionController;
		}
	}
	return nullptr;
}

// Sets default values for this component's properties
UGloveComponent::UGloveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	rotation.Init(FRotator(0, 0, 0), VRTRIX::Joint_MAX);
	FingerBendingAngle.Init(0.0, 5);
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
		for (int j= 0; j < 3; j++) {
			ml_axisoffset.M[j][i] =	LAxisOffset[i][j];
			mr_axisoffset.M[j][i] = RAxisOffset[i][j];
		}
	}
	// ...
	bIsVREnabled = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
	if (bIsVREnabled && GetTrackingSystem()) {
		GetTrackerIndex();
	};
	OnConnectGloves();
}

void UGloveComponent::EndPlay(const EEndPlayReason::Type EEndPlayReason)
{
	OnDisconnectGloves();
}

void UGloveComponent::OnReceiveNewPose(VRTRIX::Pose pose)
{
	int fingerIndex = 0;
	for (int i = 0; i < VRTRIX::Joint_MAX; ++i) {
		VRTRIX::EIMUError error = VRTRIX::IMUError_None;
		double angle = pDataGlove->GetFingerAngle((VRTRIX::Joint)i, error);
		if (error == VRTRIX::IMUError_None) {
			FingerBendingAngle[fingerIndex] = (float)angle;
			fingerIndex++;
		}

		FQuat quat = { pose.imuData[i].qx, pose.imuData[i].qy,pose.imuData[i].qz,pose.imuData[i].qw};

		FVector offset_vec = (pose.type == VRTRIX::Hand_Left) ?
			ml_axisoffset.TransformVector(FVector(quat.X, quat.Y, quat.Z)) :
			mr_axisoffset.TransformVector(FVector(quat.X, quat.Y, quat.Z)) ;
		quat = { offset_vec[0], offset_vec[1], offset_vec[2], quat.W };
		
		if (!bIsVREnabled) {
			if (pose.type == VRTRIX::Hand_Left && !bIsLOffsetCal && i == (int)VRTRIX::Wrist_Joint && quat != FQuat::Identity) {
				initialPoseOffset = LInitialPoseOffset.Quaternion() * quat.Inverse();
				bIsLOffsetCal = true;
			}
			else if (pose.type == VRTRIX::Hand_Right && !bIsROffsetCal && i == (int)VRTRIX::Wrist_Joint && quat != FQuat::Identity) {
				initialPoseOffset = RInitialPoseOffset.Quaternion() * quat.Inverse();
				bIsROffsetCal = true;
			}
		}
		else {
			if (pose.type == VRTRIX::Hand_Left  && i == (int)VRTRIX::Wrist_Joint) {
				FRotator tracker_rot;
				FVector tracker_loc;
				if (USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(m_LHTrackerIndex, tracker_loc, tracker_rot)) {
					if (!bIsLOffsetCal) {
			            LWristTrackerPitchOffset = FQuat(FVector::ForwardVector, FMath::DegreesToRadians(tracker_rot.Roll + 90.0f) );
						bIsLOffsetCal = true;
					}
					FQuat target =  tracker_rot.Quaternion() * LWristTrackerPitchOffset * LWristTrackerRotOffset.Quaternion();
					initialPoseOffset = target * quat.Inverse();
				}
			}
			else if (pose.type == VRTRIX::Hand_Right  && i == (int)VRTRIX::Wrist_Joint) {
				FRotator tracker_rot;
				FVector tracker_loc;
				if (USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(m_RHTrackerIndex, tracker_loc, tracker_rot)) {
					if (!bIsROffsetCal) {
			            RWristTrackerPitchOffset = FQuat(FVector::ForwardVector, FMath::DegreesToRadians(tracker_rot.Roll - 90.0f)); 
						bIsROffsetCal = true;
					}
					FQuat target =  tracker_rot.Quaternion() * RWristTrackerPitchOffset * RWristTrackerRotOffset.Quaternion();
					initialPoseOffset = target * quat.Inverse();
				}
			}
		}
		
		if (pose.type == VRTRIX::Hand_Left) {
			rotation[i] = (i == (int)VRTRIX::Wrist_Joint) ? 
					(initialPoseOffset * quat).Rotator() :
					(initialPoseOffset * quat * LWristFingerOffset.Quaternion()).Rotator();
		}
		else if (pose.type == VRTRIX::Hand_Right) {
			rotation[i] = (i == (int)VRTRIX::Wrist_Joint) ? 
					(initialPoseOffset * quat).Rotator() :
					(initialPoseOffset * quat * RWristFingerOffset.Quaternion()).Rotator();
		}

		if (ShowDebugInfo) {
			UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("Index %d: %f, %f, %f, %f"), i, quat.X, quat.Y, quat.Z, quat.W);
		}
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
		pDataGlove = AdvancedMode ? InitDataGlove(eInitError, VRTRIX::InitMode_Advanced, VRTRIX::DK2):
									InitDataGlove(eInitError, VRTRIX::InitMode_Normal, VRTRIX::DK2);
	}
	else if (HardwareVersion == HardwareVersion::PRO) {
		pDataGlove = AdvancedMode ? InitDataGlove(eInitError, VRTRIX::InitMode_Advanced, VRTRIX::PRO):
									InitDataGlove(eInitError, VRTRIX::InitMode_Normal, VRTRIX::PRO);
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
	//Prepare PortInfo struct and open the data streaming serial port of glove.
	VRTRIX::PortInfo portInfo = pDataGlove->IdentifyPort(eIMUError, type);

	if (eIMUError == VRTRIX::IMUError_None) {
		//Print out full port information
		UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] PORT NAME: %s"), *FString(portInfo.port.c_str()));
		UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] PORT BAUD RATE: %d"), portInfo.baud_rate);

		////Start data streaming.
		pDataGlove->StartDataStreaming(eIMUError, portInfo);
		bIsDataGloveConnected = true;
	}
	else {
		if (type == VRTRIX::Hand_Left) {
			UE_LOG(LogVRTRIXGlovePlugin, Error, TEXT("[GLOVES PULGIN] Unable to init Left Hand Glove: %d"), (int)eIMUError);
		}
		else {
			UE_LOG(LogVRTRIXGlovePlugin, Error, TEXT("[GLOVES PULGIN] Unable to init Right Hand Glove: %d"), (int)eIMUError);
		}
		pDataGlove->ClosePort(eIMUError);
		UnInit(pDataGlove);
	}
}

void UGloveComponent::OnDisconnectGloves()
{
	VRTRIX::EIMUError eIMUError;
	if (bIsDataGloveConnected) {
		pDataGlove->ClosePort(eIMUError);
		UnInit(pDataGlove);
		bIsDataGloveConnected = false;
	}
}

bool UGloveComponent::GetTrackingSystem()
{
	if (!GEngine->XRSystem.IsValid() || (GEngine->XRSystem->GetSystemName() != SteamVRSystemName))
	{
		return false;
	}

	vr::HmdError HmdErr;
	VRSystem = (vr::IVRSystem*)vr::VR_GetGenericInterface(vr::IVRSystem_Version, &HmdErr);
	VRCompositor = (vr::IVRCompositor*)vr::VR_GetGenericInterface(vr::IVRCompositor_Version, &HmdErr);

	return (VRSystem != NULL);
}
void UGloveComponent::OnTriggerHaptics()
{
	VRTRIX::EIMUError eIMUError;
	pDataGlove->VibratePeriod(eIMUError, 100);
}

void UGloveComponent::OrientationAlignment()
{
	if (type == VRTRIX::Hand_Left) {
		bIsLOffsetCal = false;
	}
	else {
		bIsROffsetCal = false;
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
	FVector offset = FVector(0,0,0);
	FRotator tracker_rot;
	FVector tracker_loc;
	IMotionController* SteamMotionController = GetSteamMotionController();
	
	switch (type) {
	case(VRTRIX::Hand_Left): {
		offset = LWristTrackerOffset;
		if (!USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(m_LHTrackerIndex, tracker_loc, tracker_rot)) {
			return FTransform::Identity;
		}
		break;
	}
	case(VRTRIX::Hand_Right): {
		offset = RWristTrackerOffset;
		if (!USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(m_RHTrackerIndex, tracker_loc, tracker_rot)) {
			return FTransform::Identity;
		}
		break;
	}
	}
	FVector new_positon = tracker_loc + tracker_rot.Quaternion() * offset;
	return FTransform(tracker_rot, new_positon, FVector(1, 1, 1));
}

EControllerHand UGloveComponent::MapHandtoEControllerHand()
{
	EControllerHand hand = EControllerHand::Special_1;
	switch (type) {
	case(VRTRIX::Hand_Left): {
		hand =  (m_LHTrackerIndex < m_RHTrackerIndex) ? EControllerHand::Special_1 : EControllerHand::Special_2;
		break;
	}
	case(VRTRIX::Hand_Right): {
		hand =  (m_LHTrackerIndex < m_RHTrackerIndex) ? EControllerHand::Special_2 : EControllerHand::Special_1;
		break;
	}
	}
	return hand;
}

// Called every frame
void UGloveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
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

//void UGloveComponent::PoseToOrientationAndPosition(const vr::HmdMatrix34_t& InPose, const float WorldToMetersScale, FQuat& OutOrientation, FVector& OutPosition) const
//{
//	FMatrix Pose = ToFMatrix(InPose);
//	if (!((FMath::Abs(1.f - Pose.GetScaledAxis(EAxis::X).SizeSquared()) <= KINDA_SMALL_NUMBER) && (FMath::Abs(1.f - Pose.GetScaledAxis(EAxis::Y).SizeSquared()) <= KINDA_SMALL_NUMBER) && (FMath::Abs(1.f - Pose.GetScaledAxis(EAxis::Z).SizeSquared()) <= KINDA_SMALL_NUMBER)))
//	{
//		// When running an oculus rift through steamvr the tracking reference seems to have a slightly scaled matrix, about 99%.  We need to strip that so we can build the quaternion without hitting an ensure.
//		Pose.RemoveScaling(KINDA_SMALL_NUMBER);
//	}
//	FQuat Orientation(Pose);
//
//	OutOrientation.X = -Orientation.Z;
//	OutOrientation.Y = Orientation.X;
//	OutOrientation.Z = Orientation.Y;
//	OutOrientation.W = -Orientation.W;
//
//	FVector Position = ((FVector(-Pose.M[3][2], Pose.M[3][0], Pose.M[3][1]) - BaseOffset) * WorldToMetersScale);
//	OutPosition = BaseOrientation.Inverse().RotateVector(Position);
//
//	OutOrientation = BaseOrientation.Inverse() * OutOrientation;
//	OutOrientation.Normalize();
//}



