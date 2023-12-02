// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class PS2LevelEditingToolsLibrary : ModuleRules
{
	public PS2LevelEditingToolsLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Cpp20;
		Type = ModuleType.External;
		PublicSystemIncludePaths.Add("$(ModuleDir)/include");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "build", "Release", "ps2-egg.lib"));
		}
	}
}
