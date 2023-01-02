#pragma once

#include "CoreMinimal.h"
#include "EditorStyleSet.h"
#include "Framework/Commands/Commands.h"

class FNodeSpawnInfo;
class UEdGraph;
class UEdGraphNode;

//////////////////////////////////////////////////////////////////////////
// FUIBlueprintEditorCommands

class FUIBlueprintEditorCommands : public TCommands<FUIBlueprintEditorCommands>
{
public:
	FUIBlueprintEditorCommands()
		: TCommands<FUIBlueprintEditorCommands>( TEXT("UIBlueprintEditor"), NSLOCTEXT("Contexts", "BlueprintEditor", "Blueprint Editor"), NAME_None, FEditorStyle::GetStyleSetName() )
	{
		
	}	

	virtual void RegisterCommands() override;
	
	// Preview commands
	TSharedPtr< FUICommandInfo > ResetCamera;
	TSharedPtr< FUICommandInfo > EnableSimulation;
	TSharedPtr< FUICommandInfo > TrackSelectedComponent;
	
	TSharedPtr< FUICommandInfo > ConvertActorSequences;
};

//////////////////////////////////////////////////////////////////////////
// FSCSUIEditorViewportCommands

class FSCSUIEditorViewportCommands : public TCommands<FSCSUIEditorViewportCommands>
{
public:
	FSCSUIEditorViewportCommands()
		: TCommands<FSCSUIEditorViewportCommands>(TEXT("SCSUIEditorViewport"), NSLOCTEXT("Contexts", "SCSEditorViewport", "SCS Editor Viewport"), NAME_None, FEditorStyle::GetStyleSetName())
	{}

	virtual void RegisterCommands() override;

	TSharedPtr< FUICommandInfo > DeleteComponent;
};
