// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BlackMyth : ModuleRules
{
	public BlackMyth(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
            "DeveloperSettings"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"BlackMyth",
			"BlackMyth/Variant_Platforming",
			"BlackMyth/Variant_Platforming/Animation",
			"BlackMyth/Variant_Combat",
			"BlackMyth/Variant_Combat/AI",
			"BlackMyth/Variant_Combat/Animation",
			"BlackMyth/Variant_Combat/Gameplay",
			"BlackMyth/Variant_Combat/Interfaces",
			"BlackMyth/Variant_Combat/UI",
			"BlackMyth/Variant_SideScrolling",
			"BlackMyth/Variant_SideScrolling/AI",
			"BlackMyth/Variant_SideScrolling/Gameplay",
			"BlackMyth/Variant_SideScrolling/Interfaces",
			"BlackMyth/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
