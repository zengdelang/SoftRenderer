using UnrealBuildTool;

public class UGUI : ModuleRules
{
    public UGUI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
                "Runtime/Renderer/Private"
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
                "RHI",
                "RenderCore",
                "Renderer",
                "InputCore",
                "ApplicationCore",
                "Projects",
                "Niagara",
                "Paper2D",
                "MovieScene",
                "MovieSceneTracks",
                "Sprite2D"
                // ... add other public dependencies that you statically link with here ...
            }
            );
            
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UMG",
                // ... add private dependencies that you statically link with here ...  
            }
            );
        
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );

        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "UnrealEd",
                    "EditorStyle",
                }
            );
        }
        
        PublicDefinitions.Add("WITH_UI_VERTEX_DATA_NORMAL_TANGENT=0");
        PublicDefinitions.Add("USE_UGUI=1");
    }
}
