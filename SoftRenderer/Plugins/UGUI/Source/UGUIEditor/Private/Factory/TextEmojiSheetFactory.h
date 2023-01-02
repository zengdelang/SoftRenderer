#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "TextEmojiSheetFactory.generated.h"

UCLASS()
class UTextEmojiSheetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UTextEmojiSheetFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
		UObject* Context, FFeedbackContext* Warn) override;
	
};
