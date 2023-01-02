#include "SoftRendererEditorModule.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserExtensions/ContentBrowserExtensions.h"
#include "ToolMenus.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/FileHelper.h"
#include "Misc/FeedbackContext.h"

void FSoftRendererEditorModule::StartupModule()
{
	if (!IsRunningCommandlet())
	{
		FSoftRendererContentBrowserExtensions::InstallHooks();
	}
}

void FSoftRendererEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FSoftRendererContentBrowserExtensions::RemoveHooks();
}

	
IMPLEMENT_MODULE(FSoftRendererEditorModule, SoftRendererEditor)
