#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

class FUISequenceEditorStyle : public FSlateStyleSet
{
public:
	FUISequenceEditorStyle()
		: FSlateStyleSet("UISequenceEditorStyle")
	{
		const FVector2D Icon16x16(16.0f, 16.0f);
		SetContentRoot(FPaths::EnginePluginsDir() / TEXT("MovieScene/ActorSequence/Content"));

		Set("ClassIcon.UISequence", new FSlateImageBrush(RootToContentDir(TEXT("ActorSequence_16x.png")), Icon16x16));
		Set("ClassIcon.UISequenceComponent", new FSlateImageBrush(RootToContentDir(TEXT("ActorSequence_16x.png")), Icon16x16));

		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

	static FUISequenceEditorStyle& Get()
	{
		static FUISequenceEditorStyle Inst;
		return Inst;
	}
	
	~FUISequenceEditorStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}
};
