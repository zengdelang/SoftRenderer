#pragma once

#include "CoreMinimal.h"
#include "Sprite2DEditor.h"
#include "UIEditorViewport/UGUIEditorViewportInfo.h"
#include "Sprite2DEditorViewportInfo.generated.h"

UCLASS()
class USprite2DEditorViewportInfo : public UUGUIEditorViewportInfo
{
	GENERATED_BODY()

public:
	TWeakPtr<FSprite2DEditor> Sprite2DEditor;
	
};
