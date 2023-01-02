using UnrealBuildTool;

public class UIBlueprintEditor : ModuleRules
{
    public UIBlueprintEditor(ReadOnlyTargetRules Target) : base(Target)
    { 
        PrivateIncludePaths.Add(EngineDirectory + "/Source/Editor/Kismet/Private");
        PrivateIncludePaths.Add("UIBlueprintEditor/Private");
        
        PrivateIncludePathModuleNames.AddRange(
            new string[] { 
                "AssetRegistry", 
                "AssetTools",
                "BlueprintRuntime",
                "ClassViewer",
                "Analytics",
                "DerivedDataCache",
                "LevelEditor",
                "GameProjectGeneration",
                "SourceCodeAccess",
            }
            );

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "AppFramework",
                "Core",
                "CoreUObject",
                "ApplicationCore",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "EditorWidgets",
                "Engine",
                "UGUI",
                "KismetCompiler",
                "UnrealEd", 
                "Kismet",
                "ToolMenus",
                "InputCore",
                "GraphEditor",
                "BlueprintGraph",
                "RenderCore",
                "RHI",
                "ViewportInteraction",
                "Paper2D",
                "Projects",
                "MovieScene",
                "MovieSceneTracks"
            }
            );

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "KismetWidgets",
            "ActorSequence",
            "Niagara",
            "RenderCore", 
            "ActorSequenceEditor",
            "UISequence",
            "UISequenceEditor",
        });
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "BlueprintRuntime",
                "ClassViewer",
                "Documentation",
                "GameProjectGeneration",
            }
            );
        
        //#zed begin : SUPPORT_UI_BLUEPRINT_EDITOR
        PublicDefinitions.Add("SUPPORT_UI_BLUEPRINT_EDITOR=1");
        //#zed end
    }
}
