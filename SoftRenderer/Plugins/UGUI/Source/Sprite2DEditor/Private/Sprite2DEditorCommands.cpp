#include "Sprite2DEditorCommands.h"

#define LOCTEXT_NAMESPACE "Sprite2DEditorCommands"

/** UI_COMMAND takes long for the compile to optimize */
PRAGMA_DISABLE_OPTIMIZATION

void FSprite2DEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenSprite2DAtlasVisualizer, "Spite Atlas Visualizer", "Open the sprite atlas visualizer.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Alt, EKeys::T));
	UI_COMMAND(OpenSprite2DAtlasPacker, "Spite Atlas Packer", "Open the sprite atlas packer.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Alt, EKeys::P));
	UI_COMMAND(ClearSprite2DAtlas, "Clear Spite Atlas", "Clear Merged Textures.", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(ClearAllSprites, "Clear All Sprites", "Clear all sprites.", EUserInterfaceActionType::Button, FInputChord());
}

PRAGMA_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
