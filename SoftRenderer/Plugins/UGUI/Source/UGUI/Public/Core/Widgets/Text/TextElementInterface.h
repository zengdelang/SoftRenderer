#pragma once

#include "CoreMinimal.h"
#include "TextAnchor.h"
#include "TextEmojiSheet.h"
#include "TextFontStyle.h"
#include "TextGenerationSettings.h"
#include "TextGenerator.h"
#include "TextWrapMode.h"
#include "Core/Widgets/Text/TextEmojiImageComponent.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Core/Renderer/UIVertex.h"
#include "UObject/Interface.h"
#include "TextElementInterface.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClickHyperlinkEvent, const FString&, Hypertext);

USTRUCT(Blueprintable)
struct FTextWidget
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	float Width;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	float Height;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	float PaddingLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	float PaddingRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	float PaddingTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	float PaddingBottom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	uint8 bScaleByFontSize : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	URectTransformComponent* WidgetRectTransform;

public:
	FTextWidget()
	{
		Width = 0;
		Height = 0;
		PaddingLeft = 0;
		PaddingRight = 0;
		PaddingTop = 0;
		PaddingBottom = 0;
		bScaleByFontSize = true;
		WidgetRectTransform = nullptr;
	}
	
	float GetScale(int32 FontSize, float Scale) const
	{
		if (bScaleByFontSize)
		{
			float FinalHeight = Height;
			if (FMath::IsNearlyZero(Height))
			{
				FinalHeight = 0.01;
			}
			return FontSize / FinalHeight * FMath::Max(0.01f, Scale);
		}
		return 1;
	}
    
	float GetWidth(float Scale) const
	{
		const float FinalWidth = Width + PaddingLeft + PaddingRight;
		if (bScaleByFontSize)
		{
			return FinalWidth * Scale;
		}
		return FinalWidth;
	}
    
	float GetHeight(float Scale) const
	{
		const float FinalHeight = Height + PaddingTop + PaddingBottom;
		if (bScaleByFontSize)
		{
			return FinalHeight * Scale;
		}
		return FinalHeight;
	}
	
};

USTRUCT(BlueprintType)
struct  FAttachWidget
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	int32 ChildWidgetIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	float PaddingLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	float PaddingRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	float PaddingTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextWidget)
	float PaddingBottom;
	
public:
	UPROPERTY(Transient)
	URectTransformComponent* AttachWidget;

public:
	FAttachWidget()
	{
		PaddingLeft = 0;
		PaddingRight = 0;
		PaddingTop = 0;
		PaddingBottom = 0;
		AttachWidget = nullptr;
	}

	float GetWidth() const
	{
		if (AttachWidget)
		{
			return AttachWidget->GetRect().Width + PaddingLeft + PaddingRight;
		}
		return 0;
	}
    
	float GetHeight() const
	{
		if (AttachWidget)
		{
			return AttachWidget->GetRect().Height + PaddingTop + PaddingBottom;
		}
		return 0;
	}
};

struct FEmojiRuntimeInfo
{
public:
	FEmojiRegion EmojiRegion;

	int32 AnimationIndex;
	float AnimationDelta;

	TWeakObjectPtr<UTextEmojiImageComponent> EmojiImageComponent;

	int32 RenderInfoKey;

public:
	FEmojiRuntimeInfo()
	{
		AnimationIndex = 0;
		AnimationDelta = 0;
		
		RenderInfoKey = 0;
	}

	UTexture* GetEmojiTexture(FTextEmoji* TextEmoji) const;
	
	void UpdateEmojiRenderInfo(FTextEmoji* TextEmoji, const FLinearColor& TextColor);
	void RemoveEmojiRenderInfo();
	
};

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API UTextElementInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class UGUI_API ITextElementInterface
{
	GENERATED_IINTERFACE_BODY()

	friend class UTextHypertextClickSubComponent;

protected:
	static TArray<FUIVertex> TempUIVertices;

	TSharedPtr<FTextGenerator> TextCache;
	TSharedPtr<FTextGenerator> TextCacheForLayout;

	TArray<FHypertextRegion> HypertextRegionList;

	TArray<FEmojiRuntimeInfo> CurEmojiList;
	
	FTimerHandle TimerHandle;
	
	float LastTimeSeconds = 0;
	
public:
	UFUNCTION(BlueprintCallable, Category = Text)
	virtual USDFFont* GetFont() const { return nullptr; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual void SetFont(USDFFont* InFont) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	FText GetText() const { return FText(); }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	void SetText(FText InText) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	bool IsSupportRichText() const { return false; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual void SetSupportRichText(bool bSupport) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual bool IsResizeTextForBestFit() const { return false; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual void SetResizeTextForBestFit(bool bResize) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual bool IsIgnoreTimeScaleForEmoji() const { return false; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual void SetIgnoreTimeScaleForEmoji(bool bInIgnoreTimeScaleForEmoji) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual int32 GetResizeTextMinSize() const { return 0; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual void SetResizeTextMinSize(int32 MinSize) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	int32 GetResizeTextMaxSize() const { return 0; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual void SetResizeTextMaxSize(int32 MaxSize) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual ETextAnchor GetAlignment() const { return ETextAnchor::TextAnchor_LowerCenter; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	void SetAlignment(ETextAnchor InAlignment) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	int32 GetFontSize() const { return 0; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual void SetFontSize(int32 InFontSize) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	EHorizontalWrapMode GetHorizontalOverflow() const { return EHorizontalWrapMode::HorizontalWrapMode_Wrap; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	void SetHorizontalOverflow(EHorizontalWrapMode InHorizontalOverflow) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	EVerticalWrapMode GetVerticalOverflow() const { return EVerticalWrapMode::VerticalWrapMode_Overflow; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	void SetVerticalOverflow(EVerticalWrapMode InVerticalOverflow) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	float GetLineSpacing() const { return 0; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	void SetLineSpacing(float InLineSpacing) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	float GetKerning() const { return 0; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual void SetKerning(float InKerning) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	float GetNonTextScale() const { return 0; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual void SetNonTextScale(float InNonTextScale) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	float GetUnderlineScale() const { return 1; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual void SetUnderlineScale(float InUnderlineScale) {}
	
	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	EFontStyle GetFontStyle() const { return EFontStyle::FontStyle_Normal; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	void SetFontStyle(EFontStyle InFontStyle) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual	bool IsBlendComponentColor() const { return false; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual void SetBlendComponentColor(bool bNewBlendComponentColor) {}

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual URectTransformComponent* GetTextTransformComponent() { return nullptr; }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual FTextWidget GetTextWidgetByName(FName InWidgetSymbol) { return FTextWidget(); }

	UFUNCTION(BlueprintCallable, Category = Text)
	virtual void SetTextWidgetByName(FName InWidgetSymbol, FTextWidget InTextWidget) {}
	
	virtual FOnClickHyperlinkEvent& GetOnClickHyperlinkEvent() = 0;
	
	virtual void SetGenerateCharacterAndLineInfo(bool bInGenerateCharacterAndLineInfo) {}

	virtual FTextGenerationSettings GetGenerationSettings(FVector2D Extents) { return FTextGenerationSettings(); }

	virtual FTextGenerator& GetCachedTextGenerator()
	{
		if (!TextCache.IsValid())
		{
			TextCache = MakeShareable(new FTextGenerator());
		}
		return *TextCache.Get();
	}

	virtual FTextGenerator& GetCachedTextGeneratorForLayout()
	{
		if (!TextCacheForLayout.IsValid())
		{
			TextCacheForLayout = MakeShareable(new FTextGenerator());
		}
		return *TextCacheForLayout.Get();
	}

	virtual TSharedPtr<FTextGenerator> GetRawCachedTextGenerator()
	{
		return TextCache;
	}

	virtual TSharedPtr<FTextGenerator> GetRawCachedTextGeneratorForLayout()
	{
		return TextCacheForLayout;
	}

	virtual FTextEmoji* GetTextEmoji(FName InEmojiSymbol) { return nullptr; }
	virtual FTextWidget* GetTextWidget(FName InWidgetSymbol) { return nullptr; }
	virtual FAttachWidget* GetAttachWidget(FName InWidgetSymbol) { return nullptr; }

};
