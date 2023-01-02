#include "Core/Widgets/Text/SDFFontCharset.h"

/////////////////////////////////////////////////////
// USDFFontCharset

USDFFontCharset::USDFFontCharset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	FontStyle = ESDFFontStyle::Normal;
	FontFilename = TEXT("Plugins/UGUI/Resources/Fonts/SourceHanSansSC-Medium.otf");
	FaceIndex = 0;
	FontScale = 36;
	PxRange = 4;
	bEnabled = true;
#endif
}

/////////////////////////////////////////////////////
