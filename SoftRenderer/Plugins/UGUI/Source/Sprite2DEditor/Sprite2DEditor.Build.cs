using UnrealBuildTool;

public class Sprite2DEditor : ModuleRules
{
    public Sprite2DEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
                "Runtime/Engine/Classes",
                "Editor/WorkspaceMenuStructure/Public"
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
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore", 
                "UGUI",
                "Sprite2D",
                "ToolMenus",
                "UIBlueprintEditor",
                "Paper2D",
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
                "EditorScriptingUtilities",
                "ApplicationCore",
                "ContentBrowser",
                // ... add private dependencies that you statically link with here ...  
            }
            );
        
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
                "DesktopPlatform",
                "MainFrame",
            }
            );
    }
}
