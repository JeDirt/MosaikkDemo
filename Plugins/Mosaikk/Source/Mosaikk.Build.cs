// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Mosaikk : ModuleRules
{
	public Mosaikk(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
			{
				"Core", 
				"Sequencer", 
				"MovieScene", 
				"MovieSceneTracks", 
				"UMGEditor",
				"MovieRenderPipelineCore", 
				"RenderCore", 
				"RHI"
			}
		);

		PrivateDependencyModuleNames.AddRange(new string[]
			{
				"Projects",
				"InputCore",
				"EditorFramework",
				"LevelEditor",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
				"LevelSequence", 
				"CinematicCamera",
				"DeveloperSettings", 
				"MovieSceneCaptureDialog"
			}
		);
	}
}
