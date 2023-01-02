#pragma once

#include "CoreMinimal.h"
#include "SDFFontEditor.h"
#include "UIEditorViewport/UGUIEditorViewportInfo.h"
#include "SDFFontEditorViewportInfo.generated.h"

UCLASS()
class USDFFontEditorViewportInfo : public UUGUIEditorViewportInfo
{
	GENERATED_BODY()

public:
	TWeakPtr<FSDFFontEditor> SDFFontEditor;
	
};
