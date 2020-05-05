// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class VRTRIXGloveLibrary : ModuleRules
{

    public VRTRIXGloveLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

        // Add any include paths for the plugin
        PublicIncludePaths.Add(ModuleDirectory);

        if (Target.Platform == UnrealTargetPlatform.Win64)
		{
            // Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "Release", "VRTRIXDATAGLOVECLIENT.lib"));
			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("VRTRIXDATAGLOVECLIENT.dll");
		}

        else if (Target.Platform == UnrealTargetPlatform.Win32)
        {
            // Add the import library
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x86", "Release", "VRTRIXDATAGLOVECLIENT.lib"));
			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("VRTRIXDATAGLOVECLIENT.dll");
        }
	}
}
