using UnrealBuildTool;

public class UGUIEditor: ModuleRules
{
    public UGUIEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
            }
            );
                
        
        PrivateIncludePaths.AddRange(
            new string[] {
                // ... add other private include paths required here ...
            }
            );
            
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "UGUI",
                "Sequencer",
                "Json",
                "JsonUtilities",
                "FreeImage",
                "ImageWrapper",
                "SharedSettingsWidgets",
                // ... add other public dependencies that you statically link with here ...
            }
            );
            
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Projects",
                "AppFramework",
                "InputCore",
                "UnrealEd",
                "LevelEditor",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "GraphEditor",
                "Kismet",
                "KismetWidgets",
                "PropertyEditor",
                "BlueprintGraph",
                "ApplicationCore",
                "Paper2D",
                "DetailCustomizations",
                "AssetTools",
                "MovieSceneTools",
                "MaterialEditor",
                "MovieScene",
                "MovieSceneTracks",
                "UIBlueprintEditor",
                // ... add private dependencies that you statically link with here ...  
            }
            );
        
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );
    }
}
