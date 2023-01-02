#pragma once

#include "CoreMinimal.h"
#include "SpriteAtlasPackerPrivate.h"
#include "Widgets/SWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Framework/Commands/UICommandList.h"

class SSpriteAtlasPackerToolbar : public SSpriteAtlasPackerBaseWidget
{
public:
	SLATE_BEGIN_ARGS(SSpriteAtlasPackerToolbar) { }
	SLATE_END_ARGS()

public:
	/**
	* Construct this widget
	*
	* @param InArgs The declaration data for this widget.
	* @param InCommandList The command list to bind to.
	*/
	void Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList);

protected:
	/**
	* Creates the toolbar widget.
	*
	* @param CommandList The command list to use for the toolbar buttons.
	* @return The toolbar widget.
	*/
	TSharedRef<SWidget> MakeToolbar(const TSharedRef<FUICommandList>& CommandList);

	void ClearAllSprites();
	
};
