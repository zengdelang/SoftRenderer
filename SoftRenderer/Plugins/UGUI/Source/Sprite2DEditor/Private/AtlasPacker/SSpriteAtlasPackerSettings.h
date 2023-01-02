#pragma once

#include "CoreMinimal.h"
#include "Sprite2D.h"
#include "SpriteAtlasPackerPrivate.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Framework/Commands/UICommandList.h"

class SSpriteAtlasPackerSettings : public SSpriteAtlasPackerBaseWidget
{
public:
	SLATE_BEGIN_ARGS(SSpriteAtlasPackerSettings) { }
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
	FReply OnImportSprites() const;

	FReply OnRemoveRedundantSprites() const;

	static FString GetRealSpriteName(FString InName);

	UTexture2D* GenerateTexture2D(const FString& ImportPath, const TSharedPtr<class FSpriteItem>& SpriteItem, TArray<UObject*>& ObjectsToSync) const;
	USprite2D* GenerateSprite2D(const FString& ImportPath, const TSharedPtr<class FSpriteItem>& SpriteItem, TArray<UObject*>& ObjectsToSync) const;
	
};
