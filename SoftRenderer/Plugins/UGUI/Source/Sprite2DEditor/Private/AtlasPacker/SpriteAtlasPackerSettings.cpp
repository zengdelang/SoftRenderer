#include "SpriteAtlasPackerSettings.h"

/////////////////////////////////////////////////////
// USpriteAtlasPackerSettings

USpriteAtlasPackerSettings::USpriteAtlasPackerSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseDynamicAtlas = true;
	PixelsPerUnrealUnit = 1;
	//ImportPath = TEXT("/Game/UI/Atlas/");
}

/////////////////////////////////////////////////////
