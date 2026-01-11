// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Lr : ModuleRules
{
	public Lr(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		//关闭优化
		OptimizeCode = CodeOptimization.Never;

		PublicDependencyModuleNames.AddRange(new string[] { "GameplayTasks","Core", "CoreUObject", "Engine", "UnrealEd", "InputCore", "EnhancedInput", "Mover", "AIModule", "GameplayTags" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Engine", "Core", "NetCore", "Networking", "PacketHandler" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}