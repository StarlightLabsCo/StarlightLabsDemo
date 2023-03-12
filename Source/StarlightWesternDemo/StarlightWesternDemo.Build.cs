// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StarlightWesternDemo : ModuleRules
{
    public StarlightWesternDemo(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "HeadMountedDisplay",
            "EnhancedInput",
            "WebSockets",
            "AudioCapture",
            "AudioMixer",
            "Json"
        });
    }
}
