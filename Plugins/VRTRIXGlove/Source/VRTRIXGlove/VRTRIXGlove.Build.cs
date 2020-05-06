// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class VRTRIXGlove : ModuleRules
{
	public VRTRIXGlove(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
                Path.Combine(ModuleDirectory, "../VRTRIXGlove/Public"),
                "Runtime/HeadMountedDisplay/Public",
				// ... add public include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "VRTRIXGloveLibrary",
				"Projects",
                "HeadMountedDisplay",
                "RHI",
                "RenderCore",
                "ProceduralMeshComponent",
                // ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64 || 
		(Target.Platform == UnrealTargetPlatform.Linux && Target.Architecture.StartsWith("x86_64")))
		{
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
            "SteamVR",
            "OpenVR",
			});
		}

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
           RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(ModuleDirectory, "../ThirdParty/VRTRIXGloveLibrary/x64/Release/", "VRTRIXDATAGLOVECLIENT.dll")));
        }

        else if (Target.Platform == UnrealTargetPlatform.Win32)
        {
          RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(ModuleDirectory, "../ThirdParty/VRTRIXGloveLibrary/x86/Release/", "VRTRIXDATAGLOVECLIENT.dll")));
        }
	}
}
