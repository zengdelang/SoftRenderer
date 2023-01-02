#include "UIEditorPerProjectUserSettings.h"
#include "UnrealEdMisc.h"

#define LOCTEXT_NAMESPACE "UIEditorPerProjectUserSettings"

UUIEditorPerProjectUserSettings::UUIEditorPerProjectUserSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRawEditMode = false;
	bShowRaycastRegion = false;
	bShowBackgroundImage = true;
    bTrackSelectedComponent = true;
	bRespectLock = true;
	bShowStats = false;
}

#if WITH_EDITOR

void UUIEditorPerProjectUserSettings::PostEditChangeProperty( FPropertyChangedEvent& PropertyChangedEvent )
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (!FUnrealEdMisc::Get().IsDeletePreferences())
	{
		SaveConfig();
	}
}

#endif

#undef LOCTEXT_NAMESPACE
