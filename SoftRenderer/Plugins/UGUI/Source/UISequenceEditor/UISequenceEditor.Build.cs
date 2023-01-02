using UnrealBuildTool;

public class UISequenceEditor : ModuleRules
{
	public UISequenceEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"UISequence",
				"BlueprintGraph",
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Kismet",
				"MovieScene",
				"MovieSceneTools",
				"Sequencer",
				"EditorStyle",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"TimeManagement"
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				"UISequenceEditor/Private",
				"UISequenceEditor/Public",
			}
		);

		var DynamicModuleNames = new string[] {
			"LevelEditor",
			"PropertyEditor",
		};

		foreach (var Name in DynamicModuleNames)
		{
			PrivateIncludePathModuleNames.Add(Name);
			DynamicallyLoadedModuleNames.Add(Name);
		}
	}
}
