#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

class FSprite2DEditorStyle : public FSlateStyleSet
{
public:
	FSprite2DEditorStyle() : FSlateStyleSet("Sprite2DEditorStyle")
	{
		
	}

	~FSprite2DEditorStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}

	void InitStyles()
	{
		const FVector2D Icon16x16(16.f, 16.f);
		const FVector2D Icon64x64(64.f, 64.f);
		const FVector2D Icon23x20(23.f, 20.f);
		const FVector2D Icon20x23(20.f, 23.f);
		const FVector2D Icon29x32(29.f, 32.f);
		
		SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("UGUI"))->GetBaseDir() / TEXT("Resources"));

		Set("Sprite2D.Trash",  new FSlateImageBrush(RootToContentDir(TEXT("Trash"), TEXT(".png")), Icon29x32));

		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

public:
	static FSprite2DEditorStyle& Get()
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
	static TOptional<FSprite2DEditorStyle> Singleton;
	
};
