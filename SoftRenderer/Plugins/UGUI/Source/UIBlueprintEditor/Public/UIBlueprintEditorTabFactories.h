#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"

#define LOCTEXT_NAMESPACE "BlueprintEditor"

/////////////////////////////////////////////////////
// FUIConstructionScriptEditorSummoner

struct FUIConstructionScriptEditorSummoner : public FWorkflowTabFactory
{
public:
	FUIConstructionScriptEditorSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp);

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;

	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override
	{
		return LOCTEXT("ComponentsTabTooltip", "The Components tab is for easily adding, selecting and attaching Components within your Blueprint.");
	}
};

/////////////////////////////////////////////////////
// FSCSUIViewportSummoner

struct FSCSUIViewportSummoner : public FWorkflowTabFactory
{
public:
	FSCSUIViewportSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp);

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual TSharedRef<SDockTab> SpawnTab(const FWorkflowTabSpawnInfo& Info) const override;
};

#undef LOCTEXT_NAMESPACE
