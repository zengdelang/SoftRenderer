#pragma once

#include "CoreMinimal.h"
#include "SCSComponentEditorExtensionContext.generated.h"

class SSCSComponentEditor;

UCLASS()
class UIBLUEPRINTEDITOR_API USCSComponentEditorExtensionContext : public UObject
{
	GENERATED_BODY()

public:
	const TWeakPtr<SSCSComponentEditor>& GetSCSComponentEditor() const { return SCSComponentEditor; }

private:
	friend SSCSComponentEditor;

	TWeakPtr<SSCSComponentEditor> SCSComponentEditor;
	
};
