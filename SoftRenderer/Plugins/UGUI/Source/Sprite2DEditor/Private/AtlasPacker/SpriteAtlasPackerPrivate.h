#pragma once

#include "CoreMinimal.h"
#include "SpriteAtlasPackerSettings.h"
#include "SSpriteAtlasPackerSpriteListItem.h"
#include "Input/Reply.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSpriteListChangedEvent, TArray<TSharedPtr<class FSpriteItem>>& NewSprites);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnSpriteListSelectionChangedEvent, int32 Width, int32 Height, UTexture2D* Texture);

struct FSpriteAtlasPackerEvents
{
	FOnSpriteListChangedEvent OnSpriteListChangedEvent;
	FOnSpriteListSelectionChangedEvent OnSpriteListSelectionChangedEvent;
};

struct SPRITE2DEDITOR_API FSpriteAtlasPacker : public FGCObject
{
	FSpriteAtlasPackerEvents& GetEvents() { return SpriteAtlasPackerEvents; }
	
	/** Static access */
	static void Initialize();
	static void Shutdown();
	static FSpriteAtlasPacker& Get();

public:
	FReply CanHandleDrag(const FDragDropEvent& DragDropEvent);
	FReply TryHandleDragDrop(const FDragDropEvent& DragDropEvent);

protected:
	bool ExistSamePath(const FString& InPath);
	bool ExistSameName(const FString& InName);
	void AddNewSprite(const FString& InPath, TArray<TSharedPtr<FSpriteItem>>& Sprites);
	bool LoadImageAsBrush(const FString& ImagePath, TSharedPtr<FSpriteItem> SpriteItem);

public:
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(Settings);
	}

	const TArray<TSharedPtr<class FSpriteItem>>& GetRootSprites()
	{
		return RootSprites;
	}

	void DeleteSelectedItem(const TSharedPtr<class FSpriteItem>& InSpriteItem)
	{
		auto& Value = SuggestedImportPathCounterMap.FindOrAdd(InSpriteItem->SuggestedImportPath);
		--Value;

		if (Value <= 0)
		{
			SuggestedImportPathCounterMap.Remove(InSpriteItem->SuggestedImportPath);
			SuggestedImportPathList.Remove(InSpriteItem->SuggestedImportPath);
		}
		
		RootSprites.Remove(InSpriteItem);

		SortSuggestedImportPathList();
	}

	void ClearRootSprites()
	{
		SuggestedImportPathCounterMap.Empty();
		SuggestedImportPathList.Empty();
		RootSprites.Empty();
	}

	USpriteAtlasPackerSettings* GetPackerSettings() const
	{
		return Settings;
	}

	const TArray<FString>& GetSuggestedImportPathList() const
	{
		return SuggestedImportPathList;
	}

private:
	void SortSuggestedImportPathList();
	
public:
	TWeakPtr<class FSpriteItem> SelectedSprite;

protected:
	TArray<TSharedPtr<class FSpriteItem>> RootSprites;

	USpriteAtlasPackerSettings* Settings = nullptr;
	
	static TSharedPtr< struct FSpriteAtlasPacker > StaticInstance;
	
	FSpriteAtlasPackerEvents SpriteAtlasPackerEvents;

	TMap<FString, int32> SuggestedImportPathCounterMap;
	TArray<FString> SuggestedImportPathList;

};

class SSpriteAtlasPackerTab : public SDockTab
{
public:
	virtual bool SupportsKeyboardFocus() const override { return true; }
};

class SSpriteAtlasPackerBaseWidget : public SCompoundWidget
{
public:
	virtual bool SupportsKeyboardFocus() const override { return true; }
};
