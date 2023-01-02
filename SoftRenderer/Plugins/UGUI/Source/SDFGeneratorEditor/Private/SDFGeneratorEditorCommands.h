#pragma once

#include "CoreMinimal.h"
#include "EditorStyleSet.h"
#include "Framework/Commands/Commands.h"

class FNodeSpawnInfo;
class UEdGraph;
class UEdGraphNode;

//////////////////////////////////////////////////////////////////////////
// FSDFGeneratorEditorCommands

class FSDFGeneratorEditorCommands : public TCommands<FSDFGeneratorEditorCommands>
{
public:
	FSDFGeneratorEditorCommands()
		: TCommands<FSDFGeneratorEditorCommands>( TEXT("SDFGeneratorEditor"), NSLOCTEXT("Contexts", "SDFGeneratorEditor", "SDF Generator Editor"), NAME_None, FEditorStyle::GetStyleSetName() )
	{
		
	}	

	virtual void RegisterCommands() override;

	TSharedPtr< FUICommandInfo > GenerateSDFFont;
	
};
