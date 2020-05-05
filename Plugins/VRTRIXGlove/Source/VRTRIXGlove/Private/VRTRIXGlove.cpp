// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VRTRIXGlove.h"
#include "Core.h"
#include "ModuleManager.h"
#include "IPluginManager.h"

#define LOCTEXT_NAMESPACE "FVRTRIXGloveModule"

void FVRTRIXGloveModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("VRTRIXGlove")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;

#if PLATFORM_WINDOWS && PLATFORM_64BITS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/VRTRIXGloveLibrary/x64/Release/VRTRIXDATAGLOVECLIENT.dll"));

#elif PLATFORM_WINDOWS 
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/VRTRIXGloveLibrary/x86/Release/VRTRIXDATAGLOVECLIENT.dll"));

#endif // PLATFORM_WINDOWS

	GloveLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (!GloveLibraryHandle)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load data glove third party library"));
	}
}

void FVRTRIXGloveModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(GloveLibraryHandle);
	GloveLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVRTRIXGloveModule, VRTRIXGlove)