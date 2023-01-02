using UnrealBuildTool;

public class UISequence : ModuleRules
{
	public UISequence(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateIncludePaths.Add("UISequence/Private");

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"MovieScene",
				"MovieSceneTracks",
				"TimeManagement",
				"UGUI"
			}
		);
	}
}
