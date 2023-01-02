using UnrealBuildTool;

public class SDFGenerator: ModuleRules
{
    protected virtual string FreeType2Version
    {
        get
        {
            if (Target.IsInPlatformGroup(UnrealPlatformGroup.Android))
            {
                return "FreeType2-2.10.4";
            }
            else if (Target.Platform == UnrealTargetPlatform.IOS ||
                     Target.Platform == UnrealTargetPlatform.Mac ||
                     Target.Platform == UnrealTargetPlatform.Win64 ||
                     Target.IsInPlatformGroup(UnrealPlatformGroup.Unix)
                    )
            {
                return "FreeType2-2.10.0";
            }
            else if (Target.Platform == UnrealTargetPlatform.TVOS)
            {
                return "FreeType2-2.4.12";
            }
            else
            {
                return "FreeType2-2.6";
            }
        }
    }

    public SDFGenerator(ReadOnlyTargetRules Target) : base(Target)
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
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
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
        
        PublicDefinitions.Add("SDF_GENERATOR_USE_OPENMP=0");
        
        if (Target.Type != TargetType.Server)
        {
            if (Target.bCompileFreeType)
            {
                AddEngineThirdPartyPrivateStaticDependencies(Target, "FreeType2");
                PublicDefinitions.Add("WITH_FREETYPE=1");
                
                if (FreeType2Version.StartsWith("FreeType2-2.10."))
                {
                    // FreeType needs these to deal with bitmap fonts
                    AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");
                    AddEngineThirdPartyPrivateStaticDependencies(Target, "UElibPNG");
                }
            }
            else
            {
                PublicDefinitions.Add("WITH_FREETYPE=0");
            }
        }
        else
        {
            PublicDefinitions.Add("WITH_FREETYPE=0");
        }
    }
}
