#include "SSpriteAtlasPackerToolbar.h"
#include "Sprite2DEditorCommands.h"
#include "Sprite2DEditorStyle.h"
#include "Framework/MultiBox/MultiBoxDefs.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "SSpriteAtlasPackerToolbar"

void SSpriteAtlasPackerToolbar::Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList)
{
	InCommandList->MapAction( FSprite2DEditorCommands::Get().ClearAllSprites,
		FUIAction( FExecuteAction::CreateSP( this, &SSpriteAtlasPackerToolbar::ClearAllSprites) ) 
		);
	
	ChildSlot
		[
			MakeToolbar(InCommandList)
		];
}

TSharedRef<SWidget> SSpriteAtlasPackerToolbar::MakeToolbar(const TSharedRef<FUICommandList>& CommandList)
{
	FToolBarBuilder ToolBarBuilder(CommandList, FMultiBoxCustomization::None);

	ToolBarBuilder.BeginSection("Debugger");
	{
		ToolBarBuilder.AddToolBarButton(FSprite2DEditorCommands::Get().ClearAllSprites, NAME_None, LOCTEXT("ClearMenu", "Clear"),
			LOCTEXT("ClearMenuTooltip", "Clear all sprites."), FSlateIcon(FSprite2DEditorStyle::Get().GetStyleSetName(), TEXT("Sprite2D.Trash")));

		//ToolBarBuilder.AddSeparator();
	}

	return ToolBarBuilder.MakeWidget();
}

void SSpriteAtlasPackerToolbar::ClearAllSprites()
{
	FSpriteAtlasPacker::Get().ClearRootSprites();

	TArray<TSharedPtr<FSpriteItem>> NewSprites;
	FSpriteAtlasPacker::Get().GetEvents().OnSpriteListChangedEvent.Broadcast(NewSprites);
}

#undef LOCTEXT_NAMESPACE
