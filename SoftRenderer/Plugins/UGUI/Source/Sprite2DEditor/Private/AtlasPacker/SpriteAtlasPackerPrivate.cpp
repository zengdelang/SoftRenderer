#include "SpriteAtlasPackerPrivate.h"
#include "Misc/FeedbackContext.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Internationalization/Regex.h"
#include "SSpriteAtlasPackerSpriteListItem.h"
#include "Misc/FileHelper.h"

#ifndef USE_DYNAMIC_UI_ALTAS
#define USE_DYNAMIC_UI_ALTAS 0
#endif

TSharedPtr< struct FSpriteAtlasPacker > FSpriteAtlasPacker::StaticInstance;

void FSpriteAtlasPacker::Initialize()
{
	StaticInstance = MakeShareable(new FSpriteAtlasPacker);
	StaticInstance->Settings = NewObject<USpriteAtlasPackerSettings>();
}

void FSpriteAtlasPacker::Shutdown()
{
	StaticInstance.Reset();
}

FSpriteAtlasPacker& FSpriteAtlasPacker::Get()
{
	return *StaticInstance;
}

FReply FSpriteAtlasPacker::CanHandleDrag(const FDragDropEvent& DragDropEvent)
{
	FReply Handled = FReply::Unhandled();

	if (TSharedPtr<FExternalDragOperation> ExternalDragDropOp = DragDropEvent.GetOperationAs<FExternalDragOperation>())
	{
		if (ExternalDragDropOp->HasFiles())
		{
			for (const auto& FilePath : ExternalDragDropOp->GetFiles())
			{
				FString Extension = FPaths::GetExtension(FilePath, false);
				if (Extension.Equals(TEXT("jpeg"), ESearchCase::IgnoreCase) || Extension.Equals(TEXT("JPG"), ESearchCase::IgnoreCase)
					|| Extension.Equals(TEXT("png"), ESearchCase::IgnoreCase) || Extension.Equals(TEXT("bmp"), ESearchCase::IgnoreCase))
				{
					return Handled = FReply::Handled();
				}
				else
				{
					if (FPaths::DirectoryExists(FilePath))
					{
						return Handled = FReply::Handled();
					}
				}
			}
		}
	}

	return Handled;
}

FReply FSpriteAtlasPacker::TryHandleDragDrop(const FDragDropEvent& DragDropEvent)
{
	TArray<TSharedPtr<FSpriteItem>> NewSprites;
	
	if (TSharedPtr<FExternalDragOperation> ExternalDragDropOp = DragDropEvent.GetOperationAs<FExternalDragOperation>())
	{
		if (ExternalDragDropOp->HasFiles())
		{
			TArray<FString> FilePathList;
			
			for (const auto& FilePath : ExternalDragDropOp->GetFiles())
			{
				if (FPaths::DirectoryExists(FilePath))
				{
					TArray<FString> FilesToAdd;
					IPlatformFile::GetPlatformPhysical().FindFilesRecursively(FilesToAdd, *FilePath, nullptr);

					for (const auto& Path : FilesToAdd)
					{
						FilePathList.Add(Path);
					}
					continue;
				}

				FilePathList.Add(FilePath);
			}

			constexpr bool bShowCancelButton = false;
			const bool bShowProgressDialog = FilePathList.Num() > 1;
			GWarn->BeginSlowTask(NSLOCTEXT("SpriteAtlasPacker", "SpriteAtlasPackerImportProgress", "Importing new sprite"), bShowProgressDialog, bShowCancelButton);

			int32 Index = 0;
			for (const auto& FilePath : FilePathList)
			{
				GWarn->UpdateProgress(Index, FilePathList.Num());
				
				++Index;
				AddNewSprite(FilePath, NewSprites);
			}

			GWarn->EndSlowTask();
		}
	}

	if (NewSprites.Num() > 0)
	{
		GetEvents().OnSpriteListChangedEvent.Broadcast(NewSprites);
	}
	
	return NewSprites.Num() > 0 ? FReply::Handled() : FReply::Unhandled();
}

bool FSpriteAtlasPacker::ExistSamePath(const FString& InPath)
{
	for (const auto& Elem : RootSprites)
	{
		if (Elem->Path == InPath)
			return true;
	}
	return false;
}

bool FSpriteAtlasPacker::ExistSameName(const FString& InName)
{
	for (const auto& Elem : RootSprites)
	{
		if (Elem->Name == InName)
			return true;
	}
	return false;
}

void FSpriteAtlasPacker::AddNewSprite(const FString& InPath, TArray<TSharedPtr<FSpriteItem>>& Sprites)
{
	const FString Extension = FPaths::GetExtension(InPath, false);
	if (Extension.Equals(TEXT("jpeg"), ESearchCase::IgnoreCase) || Extension.Equals(TEXT("JPG"), ESearchCase::IgnoreCase)
		|| Extension.Equals(TEXT("png"), ESearchCase::IgnoreCase) || Extension.Equals(TEXT("bmp"), ESearchCase::IgnoreCase))
	{
		if (ExistSamePath(InPath))
			return;
		
		const TSharedPtr<FSpriteItem> SpriteItem = MakeShareable(new FSpriteItem());

		if (!LoadImageAsBrush(InPath, SpriteItem))
			return;
		
		SpriteItem->Path = InPath;

		FString SuggestedImportPath;
		FString PathResult;

		FString ParentPath = FPaths::GetPath(InPath);
		// FString PathLeaf = FPaths::GetPathLeaf(ParentPath);

		const FRegexPattern myPattern(TEXT("(L10N)+.*UI.*"));
		FRegexMatcher Matcher(myPattern, ParentPath);
		if(Matcher.FindNext())
		{
			int32 Beginning = Matcher.GetMatchBeginning();
			int32 Ending = Matcher.GetMatchEnding();
			PathResult = ParentPath.Mid(Beginning, Ending - Beginning);
			FString PathLeaf = FPaths::GetPathLeaf(PathResult);
			while (PathLeaf != TEXT(""))
			{
				SuggestedImportPath = TEXT("/") + PathLeaf + SuggestedImportPath;
				PathResult = FPaths::GetPath(PathResult);
				PathLeaf = FPaths::GetPathLeaf(PathResult);
			}
		}
		else
		{
			const FRegexPattern myPattern_UI(TEXT("UI.*"));
			FRegexMatcher Matcher_UI(myPattern_UI, ParentPath);
			if(Matcher_UI.FindNext())
			{
				int32 Beginning = Matcher_UI.GetMatchBeginning();
				int32 Ending = Matcher_UI.GetMatchEnding();
				PathResult = ParentPath.Mid(Beginning, Ending - Beginning);
				FString PathLeaf = FPaths::GetPathLeaf(PathResult);
				while (PathLeaf != TEXT(""))
				{
					SuggestedImportPath = TEXT("/") + PathLeaf + SuggestedImportPath;
					PathResult = FPaths::GetPath(PathResult);
					PathLeaf = FPaths::GetPathLeaf(PathResult);
				}
			}
		}

		SpriteItem->SuggestedImportPath = SuggestedImportPath;

		auto& Value = SuggestedImportPathCounterMap.FindOrAdd(SuggestedImportPath);
		++Value;
		SuggestedImportPathList.AddUnique(SuggestedImportPath);

		SortSuggestedImportPathList();
		
		const FString Name = FPaths::GetBaseFilename(InPath);
		FString FinalName = Name;
		int32 Index = 1;
		while (ExistSameName(FinalName))
		{
			FinalName = Name + "_";
			FinalName.AppendInt(Index);
			++Index;
		}
		
		SpriteItem->Name = FinalName;
		
		Sprites.Add(SpriteItem);
		
		RootSprites.Add(SpriteItem);
	}
}

bool FSpriteAtlasPacker::LoadImageAsBrush(const FString& ImagePath, TSharedPtr<FSpriteItem> SpriteItem)
{
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
						const FVector2D Size = FVector2D(ImageWrapper->GetWidth(), ImageWrapper->GetHeight());
						const float Scale = 20 / FMath::Max(Size.X, Size.Y); 
						SpriteItem->ImageBrush = MakeShareable(new FSlateDynamicImageBrush(*ImagePath, Size * Scale));
						SpriteItem->Width = ImageWrapper->GetWidth();
						SpriteItem->Height = ImageWrapper->GetHeight();

						UTexture2D* Texture = NewObject<UTexture2D>(GetTransientPackage(), NAME_None, RF_Transient);
						Texture->Modify();
						
						if (Texture)
						{
							Texture->Source.Init(
								SpriteItem->Width,
								SpriteItem->Height,
								/*NumSlices=*/ 1,
								1,
								ETextureSourceFormat::TSF_BGRA8,
								RawData.GetData()
							);
						}
		
						Texture->CompressionQuality = ETextureCompressionQuality::TCQ_Highest;
						Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
						Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
						Texture->NeverStream = true;
						Texture->SRGB = true;
						Texture->CompressionSettings = TC_EditorIcon;

#if USE_DYNAMIC_UI_ALTAS
						Texture->Sprite2DTextureType = CPU;
#endif
						
						Texture->UpdateResource();

						SpriteItem->Texture = Texture;
						SpriteItem->RawData = MoveTemp(RawData);
						return true;
					}
				}
			}
		}
	}

	return false;
}

struct FCompareSuggestedImportPath
{
	TMap<FString, int32>& SuggestedImportPathCounterMap;

public:
	FCompareSuggestedImportPath(TMap<FString, int32>& InSuggestedImportPathCounterMap)
		: SuggestedImportPathCounterMap(InSuggestedImportPathCounterMap)
	{
		
	}
	
	FORCEINLINE bool operator()(const FString& A, const FString& B) const
	{
		const auto AValue = SuggestedImportPathCounterMap.FindOrAdd(A);
		const auto BValue = SuggestedImportPathCounterMap.FindOrAdd(B);
		return AValue >= BValue;
	}
};

void FSpriteAtlasPacker::SortSuggestedImportPathList()
{
	SuggestedImportPathList.Sort(FCompareSuggestedImportPath(SuggestedImportPathCounterMap));
}
