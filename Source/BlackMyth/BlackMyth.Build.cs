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
            "GameplayTasks",
			"NavigationSystem",
			"SlateCore",
            "DeveloperSettings",
            "MoviePlayer"
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

	}
}
