#include "UIBlueprintEditorTabFactories.h"
#include "BlueprintEditorTabs.h"
#include "UIBlueprintEditor.h"
#include "SSCSComponentEditor.h"
#include "SSCSUIEditorViewport.h"

#define LOCTEXT_NAMESPACE "BlueprintEditor"

/////////////////////////////////////////////////////
// FUIConstructionScriptEditorSummoner

FUIConstructionScriptEditorSummoner::FUIConstructionScriptEditorSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp)
	: FWorkflowTabFactory(FBlueprintEditorTabs::ConstructionScriptEditorID, InHostingApp)
{
	TabLabel = LOCTEXT("ComponentsTabLabel", "Components");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Tabs.Components");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("ComponentsView", "Components");
	ViewMenuTooltip = LOCTEXT("ComponentsView_ToolTip", "Show the components view");
}

TSharedRef<SWidget> FUIConstructionScriptEditorSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedPtr<FUIBlueprintEditor> UIBlueprintEditorPtr = StaticCastSharedPtr<FUIBlueprintEditor>(HostingApp.Pin());

	return UIBlueprintEditorPtr->GetSCSComponentEditor().ToSharedRef();
}

/////////////////////////////////////////////////////
// FSCSUIViewportSummoner

FSCSUIViewportSummoner::FSCSUIViewportSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp)
	: FWorkflowTabFactory(FBlueprintEditorTabs::SCSViewportID, InHostingApp)
{
	TabLabel = LOCTEXT("SCSViewportTabLabel", "Viewport");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports");

	bIsSingleton = true;
	TabRole = ETabRole::DocumentTab;

	ViewMenuDescription = LOCTEXT("SCSViewportView", "Viewport");
	ViewMenuTooltip = LOCTEXT("SCSViewportView_ToolTip", "Show the viewport view");
}

TSharedRef<SWidget> FSCSUIViewportSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedPtr<FUIBlueprintEditor> BlueprintEditorPtr = StaticCastSharedPtr<FUIBlueprintEditor>(HostingApp.Pin());

	TSharedPtr<SWidget> Result;
	if (BlueprintEditorPtr->CanAccessComponentsMode())
	{
		Result = BlueprintEditorPtr->GetSCSUIViewport();
	}

	if (Result.IsValid())
	{
		return Result.ToSharedRef();
	}
	else
	{
		return SNew(SErrorText)
			.BackgroundColor(FLinearColor::Transparent)
			.ErrorText(LOCTEXT("SCSViewportView_Unavailable", "Viewport is not available for this Blueprint."));
	}
}

TSharedRef<SDockTab> FSCSUIViewportSummoner::SpawnTab(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedRef<SDockTab> Tab = FWorkflowTabFactory::SpawnTab(Info);

	const TSharedPtr<FUIBlueprintEditor> BlueprintEditorPtr = StaticCastSharedPtr<FUIBlueprintEditor>(HostingApp.Pin());
	BlueprintEditorPtr->GetSCSUIViewport()->SetOwnerTab(Tab);

	return Tab;
}

#undef LOCTEXT_NAMESPACE
