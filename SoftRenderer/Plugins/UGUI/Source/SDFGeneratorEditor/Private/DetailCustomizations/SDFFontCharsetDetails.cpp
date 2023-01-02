#include "DetailCustomizations/SDFFontCharsetDetails.h"

#include "DesktopPlatformModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "Core/Widgets/Text/SDFFontCharset.h"
#include "FontAtlasGenerator/FontAtlasGenerator.h"

#define LOCTEXT_NAMESPACE "SDFGeneratorEditor"

TSharedRef<IDetailCustomization> FSDFFontCharsetDetails::MakeInstance()
{
	return MakeShareable(new FSDFFontCharsetDetails);
}

void FSDFFontCharsetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("FontCharset"));

	IDetailCategoryBuilder& SDFFontCharsetCategory = DetailBuilder.EditCategory(TEXT("Charset"), FText(LOCTEXT("CharsetCategory", "Charset")));

	SDFFontCharsetCategory.AddProperty(TEXT("Charset"));
	SDFFontCharsetCategory.AddProperty(TEXT("FontStyle"));
	
	const auto FontFilenameProperty = DetailBuilder.GetProperty(TEXT("FontFilename"));
	FontFilenameProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	const auto OnFontFileButtonClicked = FOnClicked::CreateLambda([&, FontFilenameProperty] ()
	{
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

		TArray<FString> OpenFileNames;
		const FString Extension = TEXT("*.ttc | *.otf | *.ttf");

		DesktopPlatform->OpenFileDialog(nullptr, TEXT("FileManager"), FPaths::ProjectDir(), "", *Extension, EFileDialogFlags::None, OpenFileNames);

		if (!OpenFileNames.IsValidIndex(0))
			return FReply::Unhandled();
		
		FString RelativeString = OpenFileNames[0];
		FPaths::MakePathRelativeTo(RelativeString, *FPaths::ProjectDir());

		FontFilenameProperty->SetValue(RelativeString);

		return FReply::Handled();
	});

	SDFFontCharsetCategory.AddCustomRow(LOCTEXT("FontFilenameRow", "Font Filename"))
	.NameContent()
	[
		FontFilenameProperty->CreatePropertyNameWidget()
	]
	.ValueContent()
	.HAlign(EHorizontalAlignment::HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.FillWidth(1)
		.VAlign(VAlign_Center)
		[
			FontFilenameProperty->CreatePropertyValueWidget()
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
			.ToolTipText(LOCTEXT("ChooseFontFileToolTipText", "Choose a font file from this computer"))
			.OnClicked(OnFontFileButtonClicked)
			.ContentPadding(2.0f)
			.ForegroundColor(FSlateColor::UseForeground())
			.IsFocusable(false)
			[
				SNew(SImage)
				.Image(FEditorStyle::Get().GetBrush("ExternalImagePicker.PickImageButton"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
	];

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<USDFFontCharset>(TargetObjects[0].Get());
	
	int32 FaceCount = 0;

	FontFaceNames.Empty();

#if WITH_FREETYPE
	if (TargetScriptPtr.IsValid())
	{
		FontAtlasGenerator::FFontHolder FontHolder;

		FString FinalFilename = TargetScriptPtr->FontFilename;
		if (!FPaths::FileExists(FinalFilename))
		{
			FString ProjectDir = FPaths::ProjectDir();
			FinalFilename = FPaths::Combine(ProjectDir, TargetScriptPtr->FontFilename);

			FString LastFinalFilename = FinalFilename;
			if (!FPaths::FileExists(FinalFilename))
			{
				ProjectDir = FPaths::GetPath(FPaths::EnginePluginsDir());
				FinalFilename = FPaths::ConvertRelativePathToFull(FPaths::Combine(ProjectDir, TargetScriptPtr->FontFilename));
				if (!FPaths::FileExists(FinalFilename))
				{
					FinalFilename = LastFinalFilename;
				}
			}
		}
	
		FaceCount = FontHolder.GetFaceCount(FinalFilename);

		FString FaceFamilyName;
		FString FaceStyleName;
		for (int32 Index = 0; Index < FaceCount; ++Index)
		{
			FontHolder.GetFaceName(FinalFilename, FaceFamilyName, FaceStyleName, Index);
			FontFaceNames.Emplace(MakeShareable (new FString(FString::Format(TEXT("{0}({1})"), {FaceFamilyName, FaceStyleName}))));
		}
	}
#endif
	
	if (TargetScriptPtr.IsValid() && FaceCount > 0)
	{
		CurrentItem = FontFaceNames[FMath::Min(FontFaceNames.Num() -1, TargetScriptPtr->FaceIndex)];
		
		FaceIndexProperty = DetailBuilder.GetProperty(TEXT("FaceIndex"));
		FaceIndexProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
		
		SDFFontCharsetCategory.AddCustomRow(LOCTEXT("FaceNameRow", "Face Name"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FaceNameTitle", "Face Name"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&FontFaceNames)
				.InitiallySelectedItem(FontFaceNames[FMath::Min(FontFaceNames.Num() -1, TargetScriptPtr->FaceIndex)])
				.OnGenerateWidget(this, &FSDFFontCharsetDetails::MakeWidgetForOption)
				.OnSelectionChanged(this, &FSDFFontCharsetDetails::OnSelectionChanged)
				[
					SNew(STextBlock)
					.Text(this, &FSDFFontCharsetDetails::GetCurrentItemLabel)
				]
			]
		];
	}
	
	SDFFontCharsetCategory.AddProperty(TEXT("FontScale"));
	SDFFontCharsetCategory.AddProperty(TEXT("PxRange"));
}

TSharedRef<SWidget> FSDFFontCharsetDetails::MakeWidgetForOption(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock).Text(FText::FromString(*InOption));
}

void FSDFFontCharsetDetails::OnSelectionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type)
{
	CurrentItem = NewValue;

	if (FaceIndexProperty.IsValid())
	{
		FaceIndexProperty->SetValue(FontFaceNames.IndexOfByKey(CurrentItem));
	}
}

FText FSDFFontCharsetDetails::GetCurrentItemLabel() const
{
	if (CurrentItem.IsValid())
	{
		return FText::FromString(*CurrentItem);
	}

	return LOCTEXT("InvalidComboEntryText", "<<Invalid option>>");
}

#undef LOCTEXT_NAMESPACE
