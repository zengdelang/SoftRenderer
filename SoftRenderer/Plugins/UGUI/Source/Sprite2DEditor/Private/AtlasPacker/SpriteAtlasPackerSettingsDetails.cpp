#include "SpriteAtlasPackerSettingsDetails.h"
#include "ContentBrowserModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "SpriteAtlasPackerSettings.h"
#include "DetailWidgetRow.h"
#include "IContentBrowserSingleton.h"
#include "SpriteAtlasPackerPrivate.h"

#define LOCTEXT_NAMESPACE "SpriteAtlasPacker"

TSharedRef<IDetailCustomization> FSpriteAtlasPackerSettingsDetails::MakeInstance()
{
	return MakeShareable(new FSpriteAtlasPackerSettingsDetails);
}

void FSpriteAtlasPackerSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Atlas"));

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<USpriteAtlasPackerSettings>(TargetObjects[0].Get());

	IDetailCategoryBuilder& SettingsCategory = DetailBuilder.EditCategory(TEXT("DynamicAtlas"), FText(LOCTEXT("AtlasCategory", "Atlas")), ECategoryPriority::Important);
	
	SettingsCategory.AddProperty(TEXT("bUseDynamicAtlas"));
	SettingsCategory.AddProperty(TEXT("AtlasName"));
	SettingsCategory.AddProperty(TEXT("PixelsPerUnrealUnit"));

	const auto ImportPathProperty = DetailBuilder.GetProperty(TEXT("ImportPath"));
	ImportPathProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	SettingsCategory.AddCustomRow(LOCTEXT("SpriteAtlasPackerSettings_ImportPath", "ImportPath"))
	.NameContent()
	[
		ImportPathProperty->CreatePropertyNameWidget()
	]
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			ImportPathProperty->CreatePropertyValueWidget()
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "ViewportMenu.Button")
			.OnClicked(this, &FSpriteAtlasPackerSettingsDetails::OnRefreshImportPath, ImportPathProperty)
			.ContentPadding(FEditorStyle::Get().GetMargin("ViewportMenu.SToolBarButtonBlock.Button.Padding"))
			.ToolTipText(LOCTEXT("AutoGenerateImportPath", "Automatically generate import path based on the import path of the assets."))
			[
				SNew(SImage)
				.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Refresh"))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 2)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "ViewportMenu.Button")
			.OnClicked(this, &FSpriteAtlasPackerSettingsDetails::OnRefreshImportPathBySelectedFolderPath, ImportPathProperty)
			.ToolTipText(LOCTEXT("ImportPathBySelectedFolderPath", "Generate import path based on the selected folder path in the content browser."))
			.ContentPadding(FEditorStyle::Get().GetMargin("ViewportMenu.SToolBarButtonBlock.Button.Padding"))
			[
				SNew(SImage)
				.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Browse"))
			]
		]
	];
}

FReply FSpriteAtlasPackerSettingsDetails::OnRefreshImportPath(TSharedRef<IPropertyHandle> Handle) const
{
	const auto Settings = FSpriteAtlasPacker::Get().GetPackerSettings();
	if (Settings)
	{
		const auto ImportPathList = FSpriteAtlasPacker::Get().GetSuggestedImportPathList();
		if (ImportPathList.Num() == 0)
			return FReply::Handled();

		FString ValidImportPath = Settings->ImportPath;
		ValidImportPath = ValidImportPath.Replace(TEXT("/Game"), TEXT(""));
		
		int32 Index = ImportPathList.Find(ValidImportPath);
		Index = (Index + 1) % ImportPathList.Num();

		if (ImportPathList.IsValidIndex(Index))
		{
			Handle->SetValue(TEXT("/Game") + ImportPathList[Index]);
		}
	}
	return FReply::Handled();
}

FReply FSpriteAtlasPackerSettingsDetails::OnRefreshImportPathBySelectedFolderPath(
	TSharedRef<IPropertyHandle> Handle) const
{
	IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>( "ContentBrowser" ).Get();

	TArray<FString> SelectFolders;
	ContentBrowser.GetSelectedFolders( SelectFolders );
	
	if (SelectFolders.Num()>0)
	{
		Handle->SetValue(SelectFolders[0]);
	}
	
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
