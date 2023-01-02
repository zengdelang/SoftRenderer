#pragma once

#include "CoreMinimal.h"
#include "EditorStyleSet.h"
#include "Framework/Commands/Commands.h"

class FNodeSpawnInfo;
class UEdGraph;
class UEdGraphNode;

//////////////////////////////////////////////////////////////////////////
// FSprite2DEditorCommands

class FSprite2DEditorCommands : public TCommands<FSprite2DEditorCommands>
{
public:
	FSprite2DEditorCommands()
		: TCommands<FSprite2DEditorCommands>( TEXT("Sprite2DEditor"), NSLOCTEXT("Contexts", "Sprite2DEditor", "Sprite2D Editor"), NAME_None, FEditorStyle::GetStyleSetName() )
	{
		
	}	

	virtual void RegisterCommands() override;
	
	TSharedPtr< FUICommandInfo > OpenSprite2DAtlasVisualizer;
	TSharedPtr< FUICommandInfo > OpenSprite2DAtlasPacker;
	TSharedPtr< FUICommandInfo > ClearSprite2DAtlas;

	TSharedPtr< FUICommandInfo > ClearAllSprites;
	
};
