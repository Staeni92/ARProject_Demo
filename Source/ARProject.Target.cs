using UnrealBuildTool;

public class ARProjectTarget : TargetRules
{
	public ARProjectTarget(TargetInfo Target) : base(Target)
	{
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		Type = TargetType.Game;

		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			bOverrideBuildEnvironment = true;
			StaticAllocator = StaticAllocatorType.Ansi;
		}

		ExtraModuleNames.Add("ARProject");
	}
}
