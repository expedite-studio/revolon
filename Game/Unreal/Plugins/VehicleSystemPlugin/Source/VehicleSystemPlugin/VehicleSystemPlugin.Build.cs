// Copyright 2019-2022 Seven47 Software. All Rights Reserved.
// Unauthorized copying of this file, via any medium is strictly prohibited

using UnrealBuildTool;

public class VehicleSystemPlugin : ModuleRules
{
	public VehicleSystemPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", });
		PrivateDependencyModuleNames.AddRange(new string[] { "Projects", "CoreUObject", "Engine", "Slate", "SlateCore", "Chaos", });

		//Required for Chaos physics callbacks
		SetupModulePhysicsSupport(Target);
	}
}
