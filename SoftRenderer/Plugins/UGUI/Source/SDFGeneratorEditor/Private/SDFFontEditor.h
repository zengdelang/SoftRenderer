#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Toolkits/IToolkitHost.h"
#include "Core/Widgets/Text/SDFFont.h"
#include "FontAtlasGenerator/FontAtlasGenerator.h"

class SSDFFontEditorTab : public SDockTab
{
public:
	virtual bool SupportsKeyboardFocus() const override { return true; }
};

class FSDFFontEditor : public FAssetEditorToolkit, public FGCObject, public FEditorUndoClient, public FontAtlasGenerator::ISDFGeneratorLogListener
{
public:
	FSDFFontEditor();
	
	/** Destructor */
	virtual ~FSDFFontEditor();

public:
	/** Edits the specified Font object */
	void InitSDFFontEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit);
	
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void SaveAsset_Execute() override;

public:
	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

protected:
	//~ Begin FEditorUndoClient Interface
	/** Handles any post undo cleanup of the GUI so that we don't have stale data being displayed. */
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override { PostUndo(bSuccess); }
	// End of FEditorUndoClient

public:
	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(SDFFont);
	}

protected:
	TSharedRef<SDockTab> SpawnTab_Charsets( const FSpawnTabArgs& Args );
	
	TSharedRef<SDockTab> SpawnTab_Viewport( const FSpawnTabArgs& Args );
	
	TSharedRef<SDockTab> SpawnTab_Details( const FSpawnTabArgs& Args );

	TSharedRef<SDockTab> SpawnTab_Preview( const FSpawnTabArgs& Args );

	TSharedRef<SDockTab> SpawnTab_MessageLog( const FSpawnTabArgs& Args );

private:
	/**	Binds our UI commands to delegates */
	void BindCommands();
	
	void CreateInternalWidgets();

	/** Extend toolbar */
	void ExtendToolbar();

	void FillToolbar(FToolBarBuilder& ToolbarBuilder);

public:
	USDFFont* GetSDFFont() const { return SDFFont; }
	TSharedPtr<class IDetailsView> GetDetailsView() const { return DetailsView; }
	FText GetPreviewText() const { return PreviewText; }
	bool IsRefreshTextComponent()
	{
		const bool bOldRefreshTextComponent = bRefreshTextComponent;
		bRefreshTextComponent = false;
		return bOldRefreshTextComponent;
	}
	
private:
	void OnPreviewTextChanged(const FText& Text);
	
	void OnGenerateSDFFont();
	bool OnGenerateSDFFontEnabled() const;

public:
	virtual void AddWarningMessage(FText Msg) override;
	virtual void AddErrorMessage(FText Msg) override;
	
	virtual void BeginSlowTask(bool bShowCancelButton, bool bShowProgressDialog) override;
    virtual void UpdateProgress(int32 CurIndex, int32 Count) override;
    virtual void EndSlowTask() override;
    		
private:
	/** The font asset being inspected */
	USDFFont* SDFFont;

	FText PreviewText;

	bool bRefreshTextComponent = false;

	TSharedPtr<class SCharsetsTabWidget> CharsetsTabWidget;

	TSharedPtr<class IDetailsView> DetailsView;

	/** Preview tab */
	TSharedPtr<SVerticalBox> FontPreview;

	TSharedPtr<class SWidget> MessageLogWidget;
	TSharedPtr<class IMessageLogListing> MessageLogListing;
	
	// Holds the list of UI commands.
	TSharedRef<FUICommandList> CommandList;

	/** Toolbar extender */
	TSharedPtr<FExtender> ToolbarExtender;
	
};

