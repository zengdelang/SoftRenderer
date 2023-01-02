#include "Factory/TextEmojiSheetFactory.h"
#include "Core/Widgets/Text/TextEmojiSheet.h"

UTextEmojiSheetFactory::UTextEmojiSheetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UTextEmojiSheet::StaticClass();
}

UObject* UTextEmojiSheetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn)
{
	UTextEmojiSheet* Asset = NewObject<UTextEmojiSheet>(InParent, InName, Flags | RF_Transactional);
	return Asset;
}