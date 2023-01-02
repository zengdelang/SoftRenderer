#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

class FUIBlueprintEditorStyle : public FSlateStyleSet
{
public:
	FUIBlueprintEditorStyle() : FSlateStyleSet("UIBlueprintEditorStyleStyle")
	{
		
	}

	~FUIBlueprintEditorStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}

	void InitStyles()
	{
		const FVector2D Icon12x12(12.f, 12.f);
		const FVector2D Icon16x16(16.f, 16.f);
		const FVector2D Icon32x32(32.f, 32.f);
		const FVector2D Icon40x40(40.f, 40.f);
		const FVector2D Icon64x64(64.f, 64.f);
		
		SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("UGUI"))->GetBaseDir() / TEXT("Resources"));

		Set("UGUI.GameObject",  new FSlateImageBrush(RootToContentDir(TEXT("Icons/GameObject@64"), TEXT(".png")), Icon64x64));
		Set("RaycastRegion", new FSlateImageBrush(RootToContentDir(TEXT("Icons/RaycastRegion@16"), TEXT(".png")), Icon16x16));
		Set("RawEditGray", new FSlateImageBrush(RootToContentDir(TEXT("Icons/RawEdit@Gray"), TEXT(".png")), Icon12x12));
		Set("RawEditBlack", new FSlateImageBrush(RootToContentDir(TEXT("Icons/RawEdit@Black"), TEXT(".png")), Icon12x12));
		Set("PreviewBackground", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Preview"), TEXT(".png")), Icon16x16));

		SetContentRoot(FPaths::EnginePluginsDir() / TEXT("MovieScene/ActorSequence/Content"));
		Set("UISequence", new FSlateImageBrush(RootToContentDir(TEXT("ActorSequence_64x.png")), Icon40x40));
		SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("UGUI"))->GetBaseDir() / TEXT("Resources"));

		Set("Refresh", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Refresh_40x"), TEXT(".png")), Icon40x40));
		
		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

public:
	static FUIBlueprintEditorStyle& Get()
	{
		if (!Singleton.IsSet())
		{
			Singleton.Emplace();
			Singleton->InitStyles();
			
		}
		return Singleton.GetValue();
	}

	static void Destroy()
	{
		Singleton.Reset();
	}

private:
	static TOptional<FUIBlueprintEditorStyle> Singleton;
	
};
