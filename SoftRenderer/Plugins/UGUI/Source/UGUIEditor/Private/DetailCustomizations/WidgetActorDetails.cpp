#include "DetailCustomizations/WidgetActorDetails.h"
#include "DesktopPlatformModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "UGUISettings.h"
#include "Core/WidgetActor.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SEnableBox.h"
#include "SResetToDefaultMenu.h"
#include "IImageWrapperModule.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FWidgetActorDetails::MakeInstance()
{
    return MakeShareable(new FWidgetActorDetails);
}

void FWidgetActorDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
    InDetailBuilder.HideCategory(TEXT("EditorPreview"));

    TArray<TWeakObjectPtr<UObject>> TargetObjects;
    InDetailBuilder.GetObjectsBeingCustomized(TargetObjects);
    TargetScriptPtr = Cast<AWidgetActor>(TargetObjects[0].Get());

	ImageBrush = LoadImageAsBrush(RelativePathToAbsolutePath(TargetScriptPtr->BackgroundImageFilePath));

    IDetailCategoryBuilder& BackgroundImageCategory = InDetailBuilder.EditCategory(TEXT("WidgetActorEditor"), FText(LOCTEXT("WidgetActorEditorCategory", "Editor")), ECategoryPriority::Important);

    auto BackgroundImageFilePathHandle = InDetailBuilder.GetProperty(TEXT("BackgroundImageFilePath"));

    const auto OnImageFileButtonClicked = FOnClicked::CreateLambda([&, BackgroundImageFilePathHandle] ()
    {
        IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

        TArray<FString> OpenFileNames;
        const FString Extension = TEXT("*.jpeg | *.png");

        DesktopPlatform->OpenFileDialog(nullptr, TEXT("FileManager"), FPaths::ProjectDir(), "", *Extension,EFileDialogFlags::Multiple, OpenFileNames);

        if (!OpenFileNames.IsValidIndex(0))
            return FReply::Unhandled();

		const FString RelativeString = GetRelativePath(OpenFileNames[0]);

		ApplyImage(RelativePathToAbsolutePath(RelativeString));

        BackgroundImageFilePathHandle->SetValue(RelativeString);

        return FReply::Handled();
    });

	const auto OnResetClicked = FSimpleDelegate::CreateLambda([&,BackgroundImageFilePathHandle]
	{
		ImageBrush->ReleaseResource();
		ImageBrush.Reset();

		BackgroundImageFilePathHandle->SetValue(FString(""));
	});

    BackgroundImageCategory.AddCustomRow(LOCTEXT("BackGroundComponent_Row", ""))
            .NameContent()
	        .MaxDesiredWidth(0.8)
            .HAlign(HAlign_Left)
               [
                   SNew(STextBlock)
                   .Font(IDetailLayoutBuilder::GetDetailFontBold())
                   .Text(LOCTEXT("BackGroundComponent_RowName", "Background Image"))
               ]
           .ValueContent()
           .HAlign(HAlign_Left)
           .VAlign(VAlign_Center)
               [
				   SNew(SHorizontalBox)
				   + SHorizontalBox::Slot()
			       .AutoWidth()
			       .VAlign(VAlign_Center)
			       [
				       SNew(SBorder)
				       .BorderImage(FEditorStyle::Get().GetBrush("ExternalImagePicker.ThumbnailShadow"))
			           .Padding(4.0f)
			           .Content()
			           [
				           SNew(SBorder)
				           .BorderImage(FEditorStyle::Get().GetBrush("ExternalImagePicker.BlankImage"))
			               .Padding(0.0f)
			               .Content()
			               [
				               SNew(SBox)
				               .WidthOverride(64)
			                   .HeightOverride(64)
			                   [
				                   SNew(SEnableBox)
				                   [
					                   SNew(SImage)
					                   .Image(this, &FWidgetActorDetails::GetImage)
				                   ]
			                   ]
			               ]
			           ]
			       ]
		           + SHorizontalBox::Slot()
			       .AutoWidth()
			       .Padding(2.0f)
			       .VAlign(VAlign_Center)
			       [
				       SNew(SButton)
				       .ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
			           .ToolTipText(LOCTEXT("ImageButtonToolTipText", "Choose a image from this computer"))
			           .OnClicked(OnImageFileButtonClicked)
			           .ContentPadding(2.0f)
			           .ForegroundColor(FSlateColor::UseForeground())
			           .IsFocusable(false)
			           [
				           SNew(SImage)
				           .Image(FEditorStyle::Get().GetBrush("ExternalImagePicker.PickImageButton"))
			                .ColorAndOpacity(FSlateColor::UseForeground())
			           ]
			       ]
				   + SHorizontalBox::Slot()
					   .AutoWidth()
					   .Padding(2.0f)
					   .VAlign(VAlign_Center)
					   [
						   SNew(SResetToDefaultMenu)
						   .OnResetToDefault(OnResetClicked)
						   .DiffersFromDefault(this,&FWidgetActorDetails::DifferFromDefault)
					   ]
			   ];
}

const FSlateBrush* FWidgetActorDetails::GetImage() const
{
	return ImageBrush.IsValid()?ImageBrush.Get(): FEditorStyle::Get().GetBrush("ExternalImagePicker.BlankImage");
}

void FWidgetActorDetails::ApplyImage(const FString& ImagePath)
{
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*ImagePath))
	{
		if(ImageBrush.IsValid())
		{
			FSlateApplication::Get().GetRenderer()->ReleaseDynamicResource(*ImageBrush.Get());
			ImageBrush.Reset();
		}

		ImageBrush = LoadImageAsBrush(ImagePath);
	}
}

TSharedPtr<FSlateDynamicImageBrush> FWidgetActorDetails::LoadImageAsBrush(const FString& ImagePath)
{
	TSharedPtr<FSlateDynamicImageBrush> Brush;
	TArray64<uint8> RawFileData;
	if (FFileHelper::LoadFileToArray(RawFileData, *ImagePath))
	{
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		TSharedPtr<IImageWrapper> ImageWrappers[4] =
		{
			ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG),
			ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP),
			ImageWrapperModule.CreateImageWrapper(EImageFormat::ICO),
			ImageWrapperModule.CreateImageWrapper(EImageFormat::ICNS),
		};

		for (auto ImageWrapper : ImageWrappers)
		{
			if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
			{
				TArray<uint8> RawData;
				if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
				{
					if (FSlateApplication::Get().GetRenderer()->GenerateDynamicImageResource(*ImagePath, ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), RawData))
					{
						Brush = MakeShareable(new FSlateDynamicImageBrush(*ImagePath, FVector2D(ImageWrapper->GetWidth(), ImageWrapper->GetHeight())));
						break;
					}
				}
			}
		}
	}

    return Brush;
}

const FString FWidgetActorDetails::RelativePathToAbsolutePath(const FString RelativePath)
{
	if(RelativePath.Contains(FString("./")) || RelativePath.Contains(FString("../")))
	{
		return FPaths::ProjectDir() + "/" + RelativePath;
	}
	else
	{
		return RelativePath;
	}
}

const FString FWidgetActorDetails::GetRelativePath(const FString RawPath)
{
	TArray<FString> RawPathStrings;
	TArray<FString> ProjectStrings;

	const FString ProjectPath = FPaths::ProjectDir();
	FString RelativePath;

	RawPath.ParseIntoArray(RawPathStrings, TEXT("/"), true);
	ProjectPath.ParseIntoArray(ProjectStrings, TEXT("/"), true);

	int32 Offset = 0;
	while(RawPathStrings[Offset] == ProjectStrings[Offset])
	{
		Offset++;
		if(!ProjectStrings.IsValidIndex(Offset))
		{
			for (int32 Index = Offset; Index < RawPathStrings.Num(); Index++)
			{
				if (Index == RawPathStrings.Num() - 1)
				{
					RelativePath += RawPathStrings[Index];
					break;
				}

				RelativePath += RawPathStrings[Index] + "/";
			}
			return "./" + RelativePath;
		}
	}

	if(Offset == 0)
	{
		return RawPath;
	}

	Offset--;

	for(int32 Index = Offset; Index < ProjectStrings.Num();Index++)
	{
		RelativePath += "../";
	}

	for(int32 Index = Offset;Index < RawPathStrings.Num();Index++)
	{
		if (Index == RawPathStrings.Num() - 1)
		{
			RelativePath += RawPathStrings[Index];
			break;
		}

		RelativePath += RawPathStrings[Index] + "/";
	}

	return RelativePath;
}

bool FWidgetActorDetails::DifferFromDefault() const
{
	return ImageBrush.IsValid();
}

#undef LOCTEXT_NAMESPACE
