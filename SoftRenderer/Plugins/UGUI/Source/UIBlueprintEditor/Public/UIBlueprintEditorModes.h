#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "BlueprintEditor.h"
#include "BlueprintEditorModes.h"
#include "WorkflowOrientedApp/ApplicationMode.h"

class UIBLUEPRINTEDITOR_API FUIBlueprintEditorUnifiedMode : public FBlueprintEditorUnifiedMode
{
public:
	FUIBlueprintEditorUnifiedMode(TSharedPtr<class FBlueprintEditor> InBlueprintEditor, FName InModeName, FText(*GetLocalizedMode)( const FName ), const bool bRegisterViewport = true);

public:
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	virtual void PreDeactivateMode() override;
	virtual void PostActivateMode() override;

protected:
	void AddUISequenceToolbar(UToolMenu* InMenu);
};
