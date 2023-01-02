#include "SDFFontFactory.h"
#include "AssetToolsModule.h"
#include "Core/Widgets/Text/SDFFont.h"

#define LOCTEXT_NAMESPACE "MsdfFontFactory"

USDFFontFactory::USDFFontFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = USDFFont::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* USDFFontFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	USDFFont* Asset = NewObject<USDFFont>(InParent, InName, Flags | RF_Transactional);
	return Asset;
}

#undef LOCTEXT_NAMESPACE