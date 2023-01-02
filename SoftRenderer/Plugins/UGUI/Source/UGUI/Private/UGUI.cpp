#include "UGUI.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FUGUIModule"

DEFINE_LOG_CATEGORY(LogUGUI);

void FUGUIModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	const FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("UGUI"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/UGUI"), PluginShaderDir);
}

void FUGUIModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUGUIModule, UGUI)
