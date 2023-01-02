#include "SDFGeneratorEditorCommands.h"

#define LOCTEXT_NAMESPACE "SDFGeneratorEditorCommands"

/** UI_COMMAND takes long for the compile to optimize */
PRAGMA_DISABLE_OPTIMIZATION

void FSDFGeneratorEditorCommands::RegisterCommands()
{
	UI_COMMAND(GenerateSDFFont, "Generate", "Generate SDF Font.", EUserInterfaceActionType::Button, FInputChord());
}

PRAGMA_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
