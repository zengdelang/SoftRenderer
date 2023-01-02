#pragma once

#include "Sprite2D.h"
#include "SSprite2DEditorViewport.h"

class SSprite2DEditorTab : public SDockTab
{
public:
	virtual bool SupportsKeyboardFocus() const override { return true; }
};

class FSprite2DEditor : public FAssetEditorToolkit, public FGCObject
{
public:
	FSprite2DEditor();
	virtual ~FSprite2DEditor() override;

public:
	void InitSprite2DEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit);

public:
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

	USprite2D* GetSprite2D() const { return Sprite; }
	TSharedPtr<class IDetailsView> GetDetailsView() const { return DetailsView; }

public:
	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

public:
	/** FGCObject interface */
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override
    {
    	Collector.AddReferencedObject(Sprite);
    }

protected:
	void CreateDetailView();
	void OnSpriteChange() const;

protected:
	TSharedRef<SDockTab> SpawnTab_Details( const FSpawnTabArgs& Args ) const;
	TSharedRef<SDockTab> SpawnTab_Viewport( const FSpawnTabArgs& Args );

private:
	/** The font asset being inspected */
	USprite2D* Sprite;
	
	// Holds the list of UI commands.
	TSharedRef<FUICommandList> CommandList;
	
	TSharedPtr<class IDetailsView> DetailsView;
	TSharedPtr<SSprite2DEditorViewport> ViewportPtr;
	
};
