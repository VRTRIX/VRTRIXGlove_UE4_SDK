// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VRTRIXGloveEditor.h"
#include "Core.h"
#include "ModuleManager.h"
#include "IPluginManager.h"

#define LOCTEXT_NAMESPACE "FVRTRIXGloveEditorModule"

void FVRTRIXGloveEditorModule::StartupModule()
{
}

void FVRTRIXGloveEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVRTRIXGloveEditorModule, VRTRIXGloveEditor)