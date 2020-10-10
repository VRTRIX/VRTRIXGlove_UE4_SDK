// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "VRTRIXDataGloveClient.h"
#include <Core.h>
#include <Engine.h>
#include "EngineUtils.h"
#include "CoreMinimal.h"

#define STEAMVR_SUPPORTED_PLATFORM (PLATFORM_LINUX || (PLATFORM_WINDOWS && WINVER > 0x0502))
// #TODO: Check this over time for when they make it global
// @TODO: hardcoded to match FSteamVRHMD::GetSystemName(), which we should turn into 
static FName SteamVRSystemName(TEXT("SteamVR"));

#if STEAMVR_SUPPORTED_PLATFORM
#include "openvr.h"
#include "ISteamVRPlugin.h"
#include "SteamVRFunctionLibrary.h"

#endif // STEAMVR_SUPPORTED_PLATFORM

#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IHeadMountedDisplay.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GloveComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVRTRIXGlovePlugin, Log, All);

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class Hand : uint8
{
	Right 	UMETA(DisplayName = "RightHand"),
	Left 	UMETA(DisplayName = "LeftHand")
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class HardwareVersion : uint8
{
	DK1 	UMETA(DisplayName = "DK1"),
	DK2 	UMETA(DisplayName = "DK2"),
	PRO		UMETA(DisplayName = "PRO"),
	PRO7	UMETA(DisplayName = "PRO7"),
	PRO11	UMETA(DisplayName = "PRO11"),
	PRO12	UMETA(DisplayName = "PRO12")
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class DeviceID : uint8
{
	Device0 	UMETA(DisplayName = "Device0"),
	Device1 	UMETA(DisplayName = "Device1"),
	Device2		UMETA(DisplayName = "Device2"),
	Device3 	UMETA(DisplayName = "Device3"),
	Device4 	UMETA(DisplayName = "Device4"),
	Device5		UMETA(DisplayName = "Device5")
};

UENUM()
enum class EBPOVRResultSwitch : uint8
{
	// On Success
	OnSucceeded,
	// On Failure
	OnFailed
};

UENUM(BlueprintType)
enum class EVRDeviceProperty_String : uint8
{

	// No prefix = 1000 series
	Prop_TrackingSystemName_String_1000				UMETA(DisplayName = "Prop_TrackingSystemName_String"),
	Prop_ModelNumber_String_1001					UMETA(DisplayName = "Prop_ModelNumber_String"),
	Prop_SerialNumber_String_1002					UMETA(DisplayName = "Prop_SerialNumber_String"),
	Prop_RenderModelName_String_1003				UMETA(DisplayName = "Prop_RenderModelName_String"),
	Prop_ManufacturerName_String_1005				UMETA(DisplayName = "Prop_ManufacturerName_String"),
	Prop_TrackingFirmwareVersion_String_1006		UMETA(DisplayName = "Prop_TrackingFirmwareVersion_String"),
	Prop_HardwareRevision_String_1007				UMETA(DisplayName = "Prop_HardwareRevision_String"),
	Prop_AllWirelessDongleDescriptions_String_1008	UMETA(DisplayName = "Prop_AllWirelessDongleDescriptions_String"),
	Prop_ConnectedWirelessDongle_String_1009		UMETA(DisplayName = "Prop_ConnectedWirelessDongle_String"),
	Prop_Firmware_ManualUpdateURL_String_1016		UMETA(DisplayName = "Prop_Firmware_ManualUpdateURL_String"),
	Prop_Firmware_ProgrammingTarget_String_1028		UMETA(DisplayName = "Prop_Firmware_ProgrammingTarget_String"),
	Prop_DriverVersion_String_1031					UMETA(DisplayName = "Prop_DriverVersion_String"),
	Prop_ResourceRoot_String_1035					UMETA(DisplayName = "Prop_ResourceRoot_String"),

	// 1 prefix = 2000 series
	HMDProp_DisplayMCImageLeft_String_2012			UMETA(DisplayName = "HMDProp_DisplayMCImageLeft_String"),
	HMDProp_DisplayMCImageRight_String_2013			UMETA(DisplayName = "HMDProp_DisplayMCImageRight_String"),
	HMDProp_DisplayGCImage_String_2021				UMETA(DisplayName = "HMDProp_DisplayGCImage_String"),
	HMDProp_CameraFirmwareDescription_String_2028	UMETA(DisplayName = "HMDProp_CameraFirmwareDescription_String"),
	HMDProp_DriverProvidedChaperonePath_String_2048 UMETA(DisplayName = "HMDProp_DriverProvidedChaperonePath_String"),

	// 2 prefix = 3000 series
	ControllerProp_AttachedDeviceId_String_3000		UMETA(DisplayName = "ControllerProp_AttachedDeviceId_String"),

	// 3 prefix = 4000 series
	TrackRefProp_ModeLabel_String_4006				UMETA(DisplayName = "TrackRefProp_ModeLabel_String"),

	// 4 prefix = 5000 series
	UIProp_NamedIconPathDeviceOff_String_5001				UMETA(DisplayName = "UIProp_NamedIconPathDeviceOff_String"),
	UIProp_NamedIconPathDeviceSearching_String_5002			UMETA(DisplayName = "UIProp_NamedIconPathDeviceSearching_String"),
	UIProp_NamedIconPathDeviceSearchingAlert_String_5003	UMETA(DisplayName = "UIProp_NamedIconPathDeviceSearchingAlert_String_"),
	UIProp_NamedIconPathDeviceReady_String_5004				UMETA(DisplayName = "UIProp_NamedIconPathDeviceReady_String"),
	UIProp_NamedIconPathDeviceReadyAlert_String_5005		UMETA(DisplayName = "UIProp_NamedIconPathDeviceReadyAlert_String"),
	UIProp_NamedIconPathDeviceNotReady_String_5006			UMETA(DisplayName = "UIProp_NamedIconPathDeviceNotReady_String"),
	UIProp_NamedIconPathDeviceStandby_String_5007			UMETA(DisplayName = "UIProp_NamedIconPathDeviceStandby_String"),
	UIProp_NamedIconPathDeviceAlertLow_String_5008			UMETA(DisplayName = "UIProp_NamedIconPathDeviceAlertLow_String"),

	// 5 prefix = 6000 series
	DriverProp_UserConfigPath_String_6000			UMETA(DisplayName = "DriverProp_UserConfigPath_String"),
	DriverProp_InstallPath_String_6001				UMETA(DisplayName = "DriverProp_InstallPath_String")

};

UENUM(BlueprintType)
enum class EVRDeviceProperty_Bool : uint8
{	
	// No prefix = 1000 series
	Prop_WillDriftInYaw_Bool_1004					UMETA(DisplayName = "Prop_WillDriftInYaw_Bool"),
	Prop_DeviceIsWireless_Bool_1010					UMETA(DisplayName = "Prop_DeviceIsWireless_Bool"),
	Prop_DeviceIsCharging_Bool_1011					UMETA(DisplayName = "Prop_DeviceIsCharging_Bool"),
	Prop_Firmware_UpdateAvailable_Bool_1014			UMETA(DisplayName = "Prop_Firmware_UpdateAvailable_Bool"),
	Prop_Firmware_ManualUpdate_Bool_1015			UMETA(DisplayName = "Prop_Firmware_ManualUpdate_Bool"),
	Prop_BlockServerShutdown_Bool_1023				UMETA(DisplayName = "Prop_BlockServerShutdown_Bool"),
	Prop_CanUnifyCoordinateSystemWithHmd_Bool_1024	UMETA(DisplayName = "Prop_CanUnifyCoordinateSystemWithHmd_Bool"),
	Prop_ContainsProximitySensor_Bool_1025			UMETA(DisplayName = "Prop_ContainsProximitySensor_Bool"),
	Prop_DeviceProvidesBatteryStatus_Bool_1026		UMETA(DisplayName = "Prop_DeviceProvidesBatteryStatus_Bool"),
	Prop_DeviceCanPowerOff_Bool_1027				UMETA(DisplayName = "Prop_DeviceCanPowerOff_Bool"),
	Prop_HasCamera_Bool_1030						UMETA(DisplayName = "Prop_HasCamera_Bool"),
	Prop_Firmware_ForceUpdateRequired_Bool_1032		UMETA(DisplayName = "Prop_Firmware_ForceUpdateRequired_Bool"),
	Prop_ViveSystemButtonFixRequired_Bool_1033		UMETA(DisplayName = "Prop_ViveSystemButtonFixRequired_Bool"),

	// 1 prefix = 2000 series
	HMDProp_ReportsTimeSinceVSync_Bool_2000				UMETA(DisplayName = "HMDProp_ReportsTimeSinceVSync_Bool"),
	HMDProp_IsOnDesktop_Bool_2007						UMETA(DisplayName = "HMDProp_IsOnDesktop_Bool"),
	HMDProp_DisplaySuppressed_Bool_2036					UMETA(DisplayName = "HMDProp_DisplaySuppressed_Bool"),
	HMDProp_DisplayAllowNightMode_Bool_2037				UMETA(DisplayName = "HMDProp_DisplayAllowNightMode_Bool"),
	HMDProp_DriverDirectModeSendsVsyncEvents_Bool_2043	UMETA(DisplayName = "HMDProp_DriverDirectModeSendsVsyncEvents_Bool"),
	HMDProp_DisplayDebugMode_Bool_2044					UMETA(DisplayName = "HMDProp_DisplayDebugMode_Bool"),


	// 5 prefix = 6000 series
	DriverProp_HasDisplayComponent_Bool_6002			UMETA(DisplayName = "DriverProp_HasDisplayComponent_Bool"),
	DriverProp_HasControllerComponent_Bool_6003			UMETA(DisplayName = "DriverProp_HasControllerComponent_Bool"),
	DriverProp_HasCameraComponent_Bool_6004				UMETA(DisplayName = "DriverProp_HasCameraComponent_Bool"),
	DriverProp_HasDriverDirectModeComponent_Bool_6005	UMETA(DisplayName = "DriverProp_HasDriverDirectModeComponent_Bool"),
	DriverProp_HasVirtualDisplayComponent_Bool_6006		UMETA(DisplayName = "DriverProp_HasVirtualDisplayComponent_Bool")

};

UENUM(BlueprintType)
enum class EVRDeviceProperty_Float : uint8
{
	// No Prefix = 1000 series
	Prop_DeviceBatteryPercentage_Float_1012						UMETA(DisplayName = "Prop_DeviceBatteryPercentage_Float"),

	// 1 Prefix = 2000 series
	HMDProp_SecondsFromVsyncToPhotons_Float_2001				UMETA(DisplayName = "HMDProp_SecondsFromVsyncToPhotons_Float"),
	HMDProp_DisplayFrequency_Float_2002							UMETA(DisplayName = "HMDProp_DisplayFrequency_Float"),
	HMDProp_UserIpdMeters_Float_2003							UMETA(DisplayName = "HMDProp_UserIpdMeters_Float"),
	HMDProp_DisplayMCOffset_Float_2009							UMETA(DisplayName = "HMDProp_DisplayMCOffset_Float"),
	HMDProp_DisplayMCScale_Float_2010							UMETA(DisplayName = "HMDProp_DisplayMCScale_Float"),
	HMDProp_DisplayGCBlackClamp_Float_2014						UMETA(DisplayName = "HMDProp_DisplayGCBlackClamp_Float"),
	HMDProp_DisplayGCOffset_Float_2018							UMETA(DisplayName = "HMDProp_DisplayGCOffset_Float"),
	HMDProp_DisplayGCScale_Float_2019							UMETA(DisplayName = "HMDProp_DisplayGCScale_Float"),
	HMDProp_DisplayGCPrescale_Float_2020						UMETA(DisplayName = "HMDProp_DisplayGCPrescale_Float"),
	HMDProp_LensCenterLeftU_Float_2022							UMETA(DisplayName = "HMDProp_LensCenterLeftU_Float"),
	HMDProp_LensCenterLeftV_Float_2023							UMETA(DisplayName = "HMDProp_LensCenterLeftV_Float"),
	HMDProp_LensCenterRightU_Float_2024							UMETA(DisplayName = "HMDProp_LensCenterRightU_Float"),
	HMDProp_LensCenterRightV_Float_2025							UMETA(DisplayName = "HMDProp_LensCenterRightV_Float"),
	HMDProp_UserHeadToEyeDepthMeters_Float_2026					UMETA(DisplayName = "HMDProp_UserHeadToEyeDepthMeters_Float"),
	HMDProp_ScreenshotHorizontalFieldOfViewDegrees_Float_2034	UMETA(DisplayName = "HMDProp_ScreenshotHorizontalFieldOfViewDegrees_Float"),
	HMDProp_ScreenshotVerticalFieldOfViewDegrees_Float_2035		UMETA(DisplayName = "HMDProp_ScreenshotVerticalFieldOfViewDegrees_Float"),
	HMDProp_SecondsFromPhotonsToVblank_Float_2042				UMETA(DisplayName = "HMDProp_SecondsFromPhotonsToVblank_Float"),

	// 3 Prefix = 4000 series
	TrackRefProp_FieldOfViewLeftDegrees_Float_4000		UMETA(DisplayName = "TrackRefProp_FieldOfViewLeftDegrees_Float"),
	TrackRefProp_FieldOfViewRightDegrees_Float_4001		UMETA(DisplayName = "TrackRefProp_FieldOfViewRightDegrees_Float"),
	TrackRefProp_FieldOfViewTopDegrees_Float_4002		UMETA(DisplayName = "TrackRefProp_FieldOfViewTopDegrees_Float"),
	TrackRefProp_FieldOfViewBottomDegrees_Float_4003	UMETA(DisplayName = "TrackRefProp_FieldOfViewBottomDegrees_Float"),
	TrackRefProp_TrackingRangeMinimumMeters_Float_4004	UMETA(DisplayName = "TrackRefProp_TrackingRangeMinimumMeters_Float"),
	TrackRefProp_TrackingRangeMaximumMeters_Float_4005	UMETA(DisplayName = "TrackRefProp_TrackingRangeMaximumMeters_Float")
};

UENUM(BlueprintType)
enum class EVRDeviceProperty_Int32 : uint8
{
	// No prefix = 1000 series
	Prop_DeviceClass_Int32_1029						UMETA(DisplayName = "Prop_DeviceClass_Int32"),

	// 1 Prefix = 2000 series
	HMDProp_DisplayMCType_Int32_2008				UMETA(DisplayName = "HMDProp_DisplayMCType_Int32"),
	HMDProp_EdidVendorID_Int32_2011					UMETA(DisplayName = "HMDProp_EdidVendorID_Int32"),
	HMDProp_EdidProductID_Int32_2015				UMETA(DisplayName = "HMDProp_EdidProductID_Int32"),
	HMDProp_DisplayGCType_Int32_2017				UMETA(DisplayName = "HMDProp_DisplayGCType_Int32"),
	HMDProp_CameraCompatibilityMode_Int32_2033		UMETA(DisplayName = "HMDProp_CameraCompatibilityMode_Int32"),
	HMDProp_DisplayMCImageWidth_Int32_2038			UMETA(DisplayName = "HMDProp_DisplayMCImageWidth_Int32"),
	HMDProp_DisplayMCImageHeight_Int32_2039			UMETA(DisplayName = "HMDProp_DisplayMCImageHeight_Int32"),
	HMDProp_DisplayMCImageNumChannels_Int32_2040	UMETA(DisplayName = "HMDProp_DisplayMCImageNumChannels_Int32"),

	// 2 Prefix = 3000 series
	ControllerProp_Axis0Type_Int32_3002				UMETA(DisplayName = "ControllerProp_Axis0Type_Int32"),
	ControllerPropProp_Axis1Type_Int32_3003			UMETA(DisplayName = "ControllerPropProp_Axis1Type_Int32"),
	ControllerPropProp_Axis2Type_Int32_3004			UMETA(DisplayName = "ControllerPropProp_Axis2Type_Int32"),
	ControllerPropProp_Axis3Type_Int32_3005			UMETA(DisplayName = "ControllerPropProp_Axis3Type_Int32"),
	ControllerPropProp_Axis4Type_Int32_3006			UMETA(DisplayName = "ControllerPropProp_Axis4Type_Int32"),
	ControllerProp_ControllerRoleHint_Int32_3007	UMETA(DisplayName = "ControllerProp_ControllerRoleHint_Int32")
};

UENUM(BlueprintType)
enum class EVRDeviceProperty_UInt64 : uint8
{
	// No prefix = 1000 series
	Prop_HardwareRevision_Uint64_1017				UMETA(DisplayName = "Prop_HardwareRevision_Uint64"),
	Prop_FirmwareVersion_Uint64_1018				UMETA(DisplayName = "Prop_FirmwareVersion_Uint64"),
	Prop_FPGAVersion_Uint64_1019					UMETA(DisplayName = "Prop_FPGAVersion_Uint64"),
	Prop_VRCVersion_Uint64_1020						UMETA(DisplayName = "Prop_VRCVersion_Uint64"),
	Prop_RadioVersion_Uint64_1021					UMETA(DisplayName = "Prop_RadioVersion_Uint64"),
	Prop_DongleVersion_Uint64_1022					UMETA(DisplayName = "Prop_DongleVersion_Uint64"),
	Prop_ParentDriver_Uint64_1034					UMETA(DisplayName = "Prop_ParentDriver_Uint64"),

	// 1 Prefix = 2000 series
	HMDProp_CurrentUniverseId_Uint64_2004			UMETA(DisplayName = "HMDProp_CurrentUniverseId_Uint64"),
	HMDProp_PreviousUniverseId_Uint64_2005			UMETA(DisplayName = "HMDProp_PreviousUniverseId_Uint64"),
	HMDProp_DisplayFirmwareVersion_Uint64_2006		UMETA(DisplayName = "HMDProp_DisplayFirmwareVersion_Uint64"),
	HMDProp_CameraFirmwareVersion_Uint64_2027		UMETA(DisplayName = "HMDProp_CameraFirmwareVersion_Uint64"),
	HMDProp_DisplayFPGAVersion_Uint64_2029			UMETA(DisplayName = "HMDProp_DisplayFPGAVersion_Uint64"),
	HMDProp_DisplayBootloaderVersion_Uint64_2030	UMETA(DisplayName = "HMDProp_DisplayBootloaderVersion_Uint64"),
	HMDProp_DisplayHardwareVersion_Uint64_2031		UMETA(DisplayName = "HMDProp_DisplayHardwareVersion_Uint64"),
	HMDProp_AudioFirmwareVersion_Uint64_2032		UMETA(DisplayName = "HMDProp_AudioFirmwareVersion_Uint64"),
	HMDProp_GraphicsAdapterLuid_Uint64_2045			UMETA(DisplayName = "HMDProp_GraphicsAdapterLuid_Uint64"),

	// 2 Prefix = 3000 series
	ControllerProp_SupportedButtons_Uint64_3001		UMETA(DisplayName = "ControllerProp_SupportedButtons_Uint64")
};

UENUM(BlueprintType)
enum class EVRDeviceProperty_Matrix34 : uint8
{
	// No prefix = 1000 series
	Prop_StatusDisplayTransform_Matrix34_1013		UMETA(DisplayName = "Prop_StatusDisplayTransform_Matrix34"),

	// 1 Prefix = 2000 series
	HMDProp_CameraToHeadTransform_Matrix34_2016		UMETA(DisplayName = "HMDProp_CameraToHeadTransform_Matrix34")
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGestureDetectDelegate);
//hand position cross the bound and fall into (Lower,Upper) will trigger the gesture event 
USTRUCT(BlueprintType, Blueprintable)//"BlueprintType" is essential to include
struct FMyGesture
{
	GENERATED_BODY()
    public:
	//The Lower Bound  position for this Gesture.Number indicate the angle between fingers and palm.Default O means don't care for the corresponding Finger. 
	//Sequence: Thumb,Index,Middle,Ring,Pinky
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyGesture")
	TArray <float> Lower_bound = {0,0,0,0,0};
	//The Upper Bound  position for this Gesture.Default O means don't care for the corresponding Finger.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyGesture")
	TArray <float> Upper_bound = { 0,0,0,0,0 };
	//Position State indicator,Blueprint accessable,True means postion is inside (Lower,Upper) 
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient, Category = "MyGesture")
	bool Gesture_State =false;
    // function to check if current hand gesture is fall into the boundaries of this gesture
	bool TriggerPositionCheck(TArray <float> Current_Position);
};


USTRUCT(BlueprintType)
struct FHandBonesName
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Wrist = "LeftHand";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Thumb1 = "LeftHandThumb1";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Thumb2 = "LeftHandThumb2";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Thumb3 = "LeftHandThumb3";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName IndexInHand = "LeftInHandIndex";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Index1 = "LeftHandIndex1";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Index2 = "LeftHandIndex2";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Index3 = "LeftHandIndex3";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName MiddleInHand = "LeftInHandMiddle";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Middle1 = "LeftHandMiddle1";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Middle2 = "LeftHandMiddle2";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Middle3 = "LeftHandMiddle3";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName RingInHand = "LeftInHandRing";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Ring1 = "LeftHandRing1";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Ring2 = "LeftHandRing2";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Ring3 = "LeftHandRing3";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName PinkyInHand = "LeftInHandPinky";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Pinky1 = "LeftHandPinky1";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Pinky2 = "LeftHandPinky2";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FName Pinky3 = "LeftHandPinky3";
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRTRIXGLOVE_API UGloveComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGloveComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EEndPlayReason)override;

public:	
	// Receive new pose from data gloves handler
	void OnReceiveNewPose(VRTRIX::Pose pose);

	// Connect data gloves
	void OnConnectGloves();
	
	// Disconnect data gloves
	void OnDisconnectGloves();

	// Initialize tracking system
	bool GetTrackingSystem();

	// Get the EControllerHand enum;
	EControllerHand MapHandtoEControllerHand();

	//Gesture detecting,update gesture state,generate gesture event
	void Calculate_Gesture_Event();

	//Gesture detecting,update gesture state
	void Calculate_Gesture_State();

	// VRTRIX Data Gloves System
	VRTRIX::IVRTRIXDataGloveClient* pDataGlove;
	


	// VRTRIX Data Gloves Port States
	bool bIsDataGlovePortOpened = false;
	
	// VRTRIX Data Gloves HandType
	VRTRIX::HandType type;

	// VRTRIX Data Gloves bone index to name map
	TMap<int, FName> BoneIndexToBoneNameMap;
	
	// Reconnect data gloves
	UFUNCTION(BlueprintCallable, Category = "VRTRIX_GLOVES")
	void ApplyHandMoCapWorldSpaceRotation(UPoseableMeshComponent *SkinMesh, FRotator alignment);

	// Reconnect data gloves
	UFUNCTION(BlueprintCallable, Category = "VRTRIX_GLOVES")
	void OnReconnect();

	//Call this function to trigger vibration for specific duration
	UFUNCTION(BlueprintCallable, Category = "VRTRIX_GLOVES")
	void OnTriggerHaptics(int duration = 100);

	//Call this function to toggle vibration (ON->OFF, OFF->ON)
	UFUNCTION(BlueprintCallable, Category = "VRTRIX_GLOVES")
	void OnToggleHaptics();

	//Allign hand's orientation  to Head Mounted Display'sorientation, only need do once after Head Mounted Display'sorientation set up changed
	UFUNCTION(BlueprintCallable, Category = "VRTRIX_GLOVES")
	void OrientationAlignment();

	//Call this function to get tracker device index and identifying lefthand tracker & righthand tracker
	UFUNCTION(BlueprintCallable, Category = "VRTRIX_GLOVES")
	void GetTrackerIndex();

	//Call this function to apply tracker offset on the wrist joint.
	UFUNCTION(BlueprintCallable, Category = "VRTRIX_GLOVES")
	FTransform ApplyTrackerOffset();

	//Call this function to get corresponded tracker transform on the wrist joint.
	UFUNCTION(BlueprintCallable, Category = "VRTRIX_GLOVES")
	FTransform GetTrackerTransform();

	//Call this function to set wrist parent joint for alignment.
	UFUNCTION(BlueprintCallable, Category = "VRTRIX_GLOVES")
	void SetWristAlignment(FRotator alignment);
	
	// Gets a String device property
	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions|SteamVR", meta = (bIgnoreSelf = "true", DisplayName = "GetVRDevicePropertyString", ExpandEnumAsExecs = "Result"))
	void GetVRDevicePropertyString(EVRDeviceProperty_String PropertyToRetrieve, int32 DeviceID, FString & StringValue, EBPOVRResultSwitch & Result);

	// Gets a Bool device property
	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions|SteamVR", meta = (bIgnoreSelf = "true", DisplayName = "GetVRDevicePropertyBool", ExpandEnumAsExecs = "Result"))
	void GetVRDevicePropertyBool(EVRDeviceProperty_Bool PropertyToRetrieve, int32 DeviceID, bool & BoolValue, EBPOVRResultSwitch & Result);

	// Gets a Float device property
	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions|SteamVR", meta = (bIgnoreSelf = "true", DisplayName = "GetVRDevicePropertyFloat", ExpandEnumAsExecs = "Result"))
	void GetVRDevicePropertyFloat(EVRDeviceProperty_Float PropertyToRetrieve, int32 DeviceID, float & FloatValue, EBPOVRResultSwitch & Result);

	// Gets a Int32 device property
	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions|SteamVR", meta = (bIgnoreSelf = "true", DisplayName = "GetVRDevicePropertyInt32", ExpandEnumAsExecs = "Result"))
	void GetVRDevicePropertyInt32(EVRDeviceProperty_Int32 PropertyToRetrieve, int32 DeviceID, int32 & IntValue, EBPOVRResultSwitch & Result);

	// Gets a UInt64 device property as a string (Blueprints do not support int64)
	UFUNCTION(BlueprintCallable, Category = "VRExpansionFunctions|SteamVR", meta = (bIgnoreSelf = "true", DisplayName = "GetVRDevicePropertyUInt64", ExpandEnumAsExecs = "Result"))
	void GetVRDevicePropertyUInt64(EVRDeviceProperty_UInt64 PropertyToRetrieve, int32 DeviceID, FString & UInt64Value, EBPOVRResultSwitch & Result);

	//Gesture Trigger Event. Hand Position Cross Into the Defined Bounds of this Gesture 
	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Triggered_0;
	//Gesture Release Event. Hand Position Cross Out the Defined Bounds of this Gesture
	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Released_0;

	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Triggered_1;
	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Released_1;

	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Triggered_2;
	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Released_2;

	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Triggered_3;
	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Released_3;

	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Triggered_4;
	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Released_4;

	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Triggered_5;
	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Released_5;

	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Triggered_6;
	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Released_6;

	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Triggered_7;
	UPROPERTY(BlueprintAssignable, Category = "Gesture_Event")
		FGestureDetectDelegate Gesture_Released_7;
	


	//Which hand you want to bond with your model,make sure the receiver for selected hand is connected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		Hand HandType = Hand::Left;
	//Which hand you want to bond with your model,make sure the receiver for selected hand is connected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		HardwareVersion HardwareVersion = HardwareVersion::PRO;
	//Server ip address to fetch glove data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FString ServerIP = "127.0.0.1";
	//Device ID to identify which glove to connect.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		DeviceID GloveID = DeviceID::Device0;
	//Advanced mode to unlock finger yaw rotation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		bool AdvancedMode = false;
	//Toggle VR Mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		bool bIsVREnabled = false;
	//Toggle Wrist Alignment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		bool bIsWristAlignEnabled = false;
	//Show debug info
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		bool ShowDebugInfo = false;
	//Rotating Coordinate offset between model and IMU.Only if using your own hand-mesh &your hand-rotation vs mesh-rotation not match, adjust this value to re-match them.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		TArray<FVector> AxisOffset = { FVector(0, 1, 0), FVector(0, 0, -1), FVector(-1, 0, 0) };
	//The original pose global rotation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FRotator InitialPoseOffset = FRotator(90, 0, 180);

	//The model thumb offset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		TArray<FVector> ThumbOffset = { FVector(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0) };
	//The model thumb proximal slerp.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		float ThumbProximalSlerp = 0.4;
	//The model thumb middle slerp.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		float ThumbMiddleSlerp = 0.7;
	//The model finger spacing when advanced mode is NOT enabled.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		float FingerSpacing = 8;
	//The model final finger spacing when four fingers are fully bended.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		float FinalFingerSpacing = 2;

	//The model finger & wrist offset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FRotator WristFingerOffset = FRotator(90, -90, 0);
	//The wrist tracker offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FVector WristTrackerOffset = FVector(-4.5f, 0, 0);
	//The wrist tracker offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		FRotator WristTrackerRotOffset = FRotator(180.0f, -90.0f, 0);

	//Customized Gestures define, gesture state will be updated every frame and can be accessed anytime.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
		TArray<FMyGesture> Gesture_NonEvent;  
         int state_gesture_num = 0;
	//Customized Gestures define. Defined Gestures will trigger the corresponding Gesture Event in Event Category.Gesture state can be accessed anytime.Up to 8 Gestures.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Developer_Configurable")
	     TArray<FMyGesture> Gesture_Event;
     	 int event_gesture_num=0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone_Configuration")
		FHandBonesName handBoneNames;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient, Category = "Glove_Data")
		TArray<FRotator> rotation;
	//Angles between fingers and palm, can be used for customized Gesture debugging. Sequence: Thumb,Index,Middle,Ring,Pinky
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient, Category = "Glove_Data")
		TArray<float> FingerBendingAngle;
	// VRTRIX Data Gloves States
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient, Category = "Glove_Data")
		bool bIsDataGloveConnected = false;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FMatrix ml_axisoffset;
	FMatrix mr_axisoffset;
	FQuat initialPoseOffset;
	FQuat alignmentPose;
	FQuat LWristTrackerPitchOffset;
	FQuat RWristTrackerPitchOffset;
	bool bIsLOffsetCal = false;
	bool bIsROffsetCal = false;
	vr::IVRSystem * VRSystem;
	vr::IVRCompositor* VRCompositor;
	FVector m_LTrackerLoc;
	FVector m_RTrackerLoc;
	FRotator m_LTrackerRot;
	FRotator m_RTrackerRot;
	int m_LHTrackerIndex;
	int m_RHTrackerIndex;
	VRTRIX::Pose m_pose;

	TArray<FVector> m_ThumbOffset = { FVector(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0) };
	float m_ThumbProximalSlerp = 0;
	float m_ThumbMiddleSlerp = 0;
	float m_FingerSpacing = 0;
	float m_FinalFingerSpacing = 0;

private:
	IMotionController* GetSteamMotionController();
	void CreateBoneIndexToBoneNameMap(FHandBonesName names);
	double CalculateBendAngle(const VRTRIX::VRTRIXQuaternion_t& q1, const VRTRIX::VRTRIXQuaternion_t& q2);
	double GetFingerBendAngle(VRTRIX::Joint finger);
	void PerformAlgorithmTuning();
};

class CVRTRIXIMUEventHandler :public VRTRIX::IVRTRIXIMUEventHandler
{
	/** OnReceivedNewPose event call back function implement
	*
	* @param pose: Pose struct returned by the call back function
	* @param pUserParam: user defined parameter
	* @returns void
	*/
	void OnReceivedNewPose(VRTRIX::Pose pose, void* pUserParam) {
		UGloveComponent* source = (UGloveComponent*)pUserParam;
		source->OnReceiveNewPose(pose);
	}

	/** OnReceivedNewEvent event call back function implement
	*
	* @param event: Event struct returned by the call back function
	* @param pUserParam: user defined parameter
	* @returns void
	*/
	void OnReceivedNewEvent(VRTRIX::HandEvent event, void* pUserParam) {
		UGloveComponent* source = (UGloveComponent*)pUserParam;
		FString handTypeString = (event.type == VRTRIX::Hand_Left) ? "Left Hand Glove" : "Right Hand Glove";
 		switch (event.stat) {
		case(VRTRIX::HandStatus_Connected): {
			UE_LOG(LogVRTRIXGlovePlugin, Warning, TEXT("[GLOVES PULGIN] %s Connected at address: %s: %s."), *handTypeString, *source->ServerIP, *(FString::FromInt(11002+(int)source->GloveID)));
			source->bIsDataGloveConnected = true;

			//Set radio channel limit between 65 to 99 (2465Mhz to 2499Mhz) before start data streaming if needed. (this step is optional)
			VRTRIX::EIMUError eIMUError;
			source->pDataGlove->SetRadioChannelLimit(eIMUError, 99, 65);
			if (eIMUError == VRTRIX::IMUError_DataNotValid) UE_LOG(LogVRTRIXGlovePlugin, Display, TEXT("[GLOVES PULGIN] Radio Channel Not Valid!"));
			break;
		}
		case(VRTRIX::HandStatus_Disconnected): {
			UE_LOG(LogVRTRIXGlovePlugin, Warning, TEXT("[GLOVES PULGIN] %s Disconnected."), *handTypeString);
			source->bIsDataGloveConnected = false;
			break;
		}
		}
	}
};

