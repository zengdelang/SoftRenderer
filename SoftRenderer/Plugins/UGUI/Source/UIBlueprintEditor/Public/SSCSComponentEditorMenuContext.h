#pragma once

#include "CoreMinimal.h"
#include "SSCSComponentEditorMenuContext.generated.h"

class SSCSComponentEditor;

UCLASS()
class UIBLUEPRINTEDITOR_API USSCSComponentEditorMenuContext : public UObject
{
	GENERATED_BODY()
	
public:
	TWeakPtr<SSCSComponentEditor> SCSComponentEditor;
	bool bOnlyShowPasteOption;
	
};
