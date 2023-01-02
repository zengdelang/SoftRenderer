#pragma once

#include "CoreMinimal.h"
#include "SUGUIEditorViewport.h"
#include "UGUIEditorViewportClient.h"
#include "UGUIEditorViewportInfo.generated.h"

UCLASS()
class UIBLUEPRINTEDITOR_API UUGUIEditorViewportInfo : public UObject
{
	GENERATED_BODY()

public:
	FName ViewportName;

	TWeakPtr<FUGUIEditorViewportClient> ViewportClient;

	TWeakPtr<SUGUIEditorViewport> EditorViewport;
	
};
