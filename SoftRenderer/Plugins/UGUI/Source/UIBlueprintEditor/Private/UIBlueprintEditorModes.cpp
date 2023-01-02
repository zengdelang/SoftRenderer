#include "UIBlueprintEditorModes.h"
#include "BlueprintEditorContext.h"
#include "BlueprintEditorTabs.h"
#include "SBlueprintEditorToolbar.h"
#include "ToolMenu.h"
#include "ToolMenus.h"
#include "UIBlueprintEditorCommands.h"
#include "UIBlueprintEditorStyle.h"
#include "UIBlueprintEditorTabFactories.h"

#define LOCTEXT_NAMESPACE "BlueprintEditor"

////////////////////////////////////////
// FUIBlueprintEditorUnifiedMode

FUIBlueprintEditorUnifiedMode::FUIBlueprintEditorUnifiedMode(TSharedPtr<class FBlueprintEditor> InBlueprintEditor, FName InModeName, FText(*GetLocalizedMode)( const FName ), const bool bRegisterViewport)
	: FBlueprintEditorUnifiedMode(InBlueprintEditor, InModeName, GetLocalizedMode, bRegisterViewport)
{
	if (bRegisterViewport)
	{
		BlueprintEditorTabFactories.UnregisterFactory(FBlueprintEditorTabs::ConstructionScriptEditorID);
		BlueprintEditorTabFactories.RegisterFactory(MakeShareable(new FUIConstructionScriptEditorSummoner(InBlueprintEditor)));

		BlueprintEditorTabFactories.UnregisterFactory(FBlueprintEditorTabs::SCSViewportID);
		BlueprintEditorTabFactories.RegisterFactory(MakeShareable(new FSCSUIViewportSummoner(InBlueprintEditor)));
	}

	FName ParentToolbarName;
	const FName ModeSpecificToolbarName = InBlueprintEditor->GetToolMenuToolbarNameForMode(InModeName, ParentToolbarName);
	if (UToolMenu* Toolbar = UToolMenus::Get()->FindMenu(ModeSpecificToolbarName))
	{
		AddUISequenceToolbar(Toolbar);
	}
}

void FUIBlueprintEditorUnifiedMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	FBlueprintEditorUnifiedMode::RegisterTabFactories(InTabManager);
}

void FUIBlueprintEditorUnifiedMode::PreDeactivateMode()
{
	FBlueprintEditorUnifiedMode::PreDeactivateMode();
}

void FUIBlueprintEditorUnifiedMode::PostActivateMode()
{
	FBlueprintEditorUnifiedMode::PostActivateMode();
}

void FUIBlueprintEditorUnifiedMode::AddUISequenceToolbar(UToolMenu* InMenu)
{
	FToolMenuSection& Section = InMenu->AddSection("UISequence");
	Section.InsertPosition = FToolMenuInsert("Debugging", EToolMenuInsertType::After);

	Section.AddDynamicEntry("UISequenceCommands", FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
	{
		UBlueprintEditorToolMenuContext* Context = InSection.FindContext<UBlueprintEditorToolMenuContext>();
		if (Context && Context->BlueprintEditor.IsValid() && Context->GetBlueprintObj())
		{
			TSharedPtr<class FBlueprintEditorToolbar> BlueprintEditorToolbar = Context->BlueprintEditor.Pin()->GetToolbarBuilder();
			if (BlueprintEditorToolbar.IsValid())
			{
				const FUIBlueprintEditorCommands& Commands = FUIBlueprintEditorCommands::Get();
				
				InSection.AddEntry(FToolMenuEntry::InitToolBarButton(
					Commands.ConvertActorSequences,
					TAttribute<FText>(),
					TAttribute<FText>(LOCTEXT("ConvertActorSequences_Tooltip", "Convert all actor sequences to UI sequences")),
					TAttribute<FSlateIcon>(FSlateIcon(FUIBlueprintEditorStyle::Get().GetStyleSetName(), "UISequence")),
					"ConvertActorSequences"
				));
			}
		}
	}));
}

#undef LOCTEXT_NAMESPACE
