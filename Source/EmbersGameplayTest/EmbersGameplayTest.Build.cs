// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EmbersGameplayTest : ModuleRules
{
	public EmbersGameplayTest(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Niagara" });

		PublicIncludePaths.AddRange(new string[] {
			"EmbersGameplayTest",
			"EmbersGameplayTest/Interaction",
			//"EmbersGameplayTest/Variant_Horror",
			//"EmbersGameplayTest/Variant_Horror/UI",
			//"EmbersGameplayTest/Variant_Shooter",
			//"EmbersGameplayTest/Variant_Shooter/AI",
			//"EmbersGameplayTest/Variant_Shooter/UI",
			//"EmbersGameplayTest/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
