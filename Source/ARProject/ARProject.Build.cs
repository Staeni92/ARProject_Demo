using UnrealBuildTool;

public class ARProject : ModuleRules
{
	public ARProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AugmentedReality",
			"ProceduralMeshComponent",
			"UMG",
			"Slate",
			"SlateCore",
			"MediaAssets",
			"AudioMixer",
			"ImageWrapper",
			"RenderCore"
		});
	}
}
