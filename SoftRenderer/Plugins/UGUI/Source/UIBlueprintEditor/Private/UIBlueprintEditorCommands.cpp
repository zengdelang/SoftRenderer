#include "UIBlueprintEditorCommands.h"

#define LOCTEXT_NAMESPACE "BlueprintEditorCommands"

/** UI_COMMAND takes long for the compile to optimize */
PRAGMA_DISABLE_OPTIMIZATION

void FUIBlueprintEditorCommands::RegisterCommands()
{
	// Preview commands
	UI_COMMAND( ResetCamera, "Reset Camera", "Resets the camera to focus on the mesh", EUserInterfaceActionType::Button, FInputChord() );
    UI_COMMAND( EnableSimulation, "Simulation", "Enables simulation of components in the viewport", EUserInterfaceActionType::ToggleButton, FInputChord() );
	UI_COMMAND( TrackSelectedComponent, "Track Selected Component", "Track selected component", EUserInterfaceActionType::ToggleButton, FInputChord() );

	UI_COMMAND( ConvertActorSequences, "Convert Sequences", "Convert all actor sequences to UI sequences", EUserInterfaceActionType::Button, FInputChord());
}

PRAGMA_ENABLE_OPTIMIZATION

//////////////////////////////////////////////////////////////////////////
// FSCSUIEditorViewportCommands

void FSCSUIEditorViewportCommands::RegisterCommands()
{
	UI_COMMAND(DeleteComponent, "Delete", "Delete current selection", EUserInterfaceActionType::Button, FInputChord(EKeys::Platform_Delete));
}

#undef LOCTEXT_NAMESPACE
