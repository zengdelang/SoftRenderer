#include "Sprite2DModule.h"

#define LOCTEXT_NAMESPACE "FSprite2DModule"

DEFINE_LOG_CATEGORY(LogSprite2D);

void FSprite2DModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FSprite2DModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSprite2DModule, Sprite2D)
