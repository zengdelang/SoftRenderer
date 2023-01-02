#pragma once

#include "CoreMinimal.h"
#include "TextWrapMode.h"
#include "TextFontStyle.h"
#include "TextGenerationSettings.h"
#include "Core/Widgets/MaskableGraphicSubComponent.h"
#include "TextElementInterface.h"
#include "TextEmojiSheet.h"
#include "Core/Layout/LayoutElementInterface.h"
#include "TextSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Text"))
class UGUI_API UTextSubComponent : public UMaskableGraphicSubComponent, public ILayoutElementInterface, public ITextElementInterface
{
	GENERATED_UCLASS_BODY()
	
protected:
	/**
	 * Text that's being displayed by the Text.
	 *
	 * This is the string value of a Text component. Use this to read or edit the message displayed in Text.
	 */
	UPROPERTY(EditAnywhere, Category = Content, meta = (MultiLine = "true"))
	FText Text;

	/**
	 * The Font used by the text.
	 *
	 * This is the font used by the Text component. Use it to alter or return the font from the Text. There are many free fonts available online.
	 */
	UPROPERTY(EditAnywhere, Category = Appearance)
	USDFFont* Font;

	/**
	 * Whether this Text will support rich text.
	 */
	UPROPERTY(EditAnywhere, Category = Appearance)
	uint8 bSupportRichText : 1;

	/**
	 * Should the text be allowed to auto resized.
	 */
	UPROPERTY(EditAnywhere, Category = Appearance)
	uint8 bResizeTextForBestFit : 1;

	UPROPERTY(EditAnywhere, Category = Appearance)
	uint8 bIgnoreTimeScaleForEmoji : 1;

	UPROPERTY(EditAnywhere, Category = Appearance)
	uint8 bElipsizeEnd : 1;

	UPROPERTY(EditAnywhere, Category = Appearance)
	uint8 bBlendComponentColor: 1 ;

	uint8 bGenerateCharacterAndLineInfo : 1;

	uint8 bNeedSetTimer : 1;

	/**
	 * The minimum size the text is allowed to be.
	 */
	UPROPERTY(EditAnywhere, Category = Appearance, meta = (ClampMin = "1", ClampMax = "300", UIMin = "1", UIMax = "300"))
	int32 ResizeTextMinSize;

	/**
	 * The maximum size the text is allowed to be. 1 = infinitely large.
	 */
	UPROPERTY(EditAnywhere, Category = Appearance, meta = (ClampMin = "1", ClampMax = "300", UIMin = "1", UIMax = "300"))
	int32 ResizeTextMaxSize;

	/**
	 * The positioning of the text relative to its [[RectTransform]].
	 *
	 * This is the positioning of the Text relative to its RectTransform. You can alter this via script or in the Inspector of a Text component using the buttons in the Alignment section.
	 */
	UPROPERTY(EditAnywhere, Category = Appearance)
	ETextAnchor Alignment;

	/**
	 * The size that the Font should render at.
	 */
	UPROPERTY(EditAnywhere, Category = Appearance, meta = (ClampMin = "1", ClampMax = "300", UIMin = "1", UIMax = "300"))
	int32 FontSize;

	/**
	 * Horizontal overflow mode.
	 *
	 * When set to HorizontalWrapMode.Overflow, text can exceed the horizontal boundaries of the Text graphic. When set to HorizontalWrapMode.Wrap, text will be word-wrapped to fit within the boundaries.
	 */
	UPROPERTY(EditAnywhere, Category = Appearance)
	EHorizontalWrapMode HorizontalOverflow;

	/**
	 * Vertical overflow mode.
	 */
	UPROPERTY(EditAnywhere, Category = Appearance)
	EVerticalWrapMode VerticalOverflow;

	/**
	 * Line spacing, specified as a factor of font line height. A value of 1 will produce normal line spacing.
	 */
	UPROPERTY(EditAnywhere, Category = Appearance)
	float LineSpacing;

	UPROPERTY(EditAnywhere, Category = Appearance)
	float Kerning;

	UPROPERTY(EditAnywhere, Category = Appearance, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float NonTextScale;
	
	UPROPERTY(EditAnywhere, Category = Apperance, meta = (ClampMin = "0", UIMIN = "0"))
	float UnderlineScale;
	
	/**
	 * Font style used by the Text's text.
	 */
	UPROPERTY(EditAnywhere, Category = Appearance)
	EFontStyle FontStyle;

	UPROPERTY(EditAnywhere, Category = Appearance)
	TMap<FName, FTextWidget> TextWidgets;

	UPROPERTY(EditAnywhere, Category = Appearance)
	TMap<FName, FAttachWidget> AttachWidgets;
	
	UPROPERTY(EditAnywhere, Category = Appearance)
	TMap<FName, FTextEmoji> TextEmojis;

	UPROPERTY(EditAnywhere, Category = Appearance)
	TArray<UTextEmojiSheet*> TextEmojiSheets;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnClickHyperlinkEvent OnHyperlinkClick;
	
protected:
	UPROPERTY(Transient)
	TArray<UTextEmojiImageComponent*> EmojiImages;
	
	TMap<TWeakObjectPtr<UTexture>, TWeakObjectPtr<UTextEmojiImageComponent>> UsedEmojiImages;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	TWeakObjectPtr<UTextEmojiImageComponent> DebugEmojiImage;
#endif

	TMap<FName, FDelegateHandle> AttachWidgetRectChangeDelegateHandles;
	
public:
	virtual UTexture* GetMainTexture() const override 
	{
		if (Font && Font->FontTextureArray)
			return Font->FontTextureArray;
		return nullptr;
	}

public: 
	virtual USDFFont* GetFont() const override
	{
		return Font;
	}

	virtual void SetFont(USDFFont* InFont) override
	{
		if (Font != InFont)
		{
			Font = InFont;
			SetAllDirty();
		}
	}

	virtual FText GetText() const override
	{
		return Text;
	}

	virtual void SetText(FText InText) override;

	virtual bool IsSupportRichText() const override
	{
		return bSupportRichText;
	}

	virtual void SetSupportRichText(bool bSupport) override
	{
		if (bSupportRichText == bSupport)
			return;
		
		bSupportRichText = bSupport;
		SetVerticesDirty();
		SetLayoutDirty();
	}

	virtual bool IsResizeTextForBestFit() const override
	{
		return bResizeTextForBestFit;
	}

	virtual void SetResizeTextForBestFit(bool bResize) override
	{
		if (bResizeTextForBestFit == bResize)
			return;
		
		bResizeTextForBestFit = bResize;
		SetVerticesDirty();
		SetLayoutDirty();
	}

	virtual bool IsIgnoreTimeScaleForEmoji() const override
	{
		return bIgnoreTimeScaleForEmoji;
	}

	virtual void SetIgnoreTimeScaleForEmoji(bool bInIgnoreTimeScaleForEmoji) override
	{
		bIgnoreTimeScaleForEmoji = bInIgnoreTimeScaleForEmoji;
	}

	virtual int32 GetResizeTextMinSize() const override
	{
		return ResizeTextMinSize;
	}

	virtual void SetResizeTextMinSize(int32 MinSize) override
	{
		MinSize = FMath::Clamp(MinSize, 0, FontSize);
		if (ResizeTextMinSize == MinSize)
			return;
		
		ResizeTextMinSize = MinSize;
		SetVerticesDirty();
		SetLayoutDirty();
	}

	virtual int32 GetResizeTextMaxSize() const override
	{
		return ResizeTextMaxSize;
	}

	virtual void SetResizeTextMaxSize(int32 MaxSize) override
	{
		MaxSize = FMath::Clamp(MaxSize, FontSize, 300);
		if (ResizeTextMaxSize == MaxSize)
			return;
		
		ResizeTextMaxSize = MaxSize;
		SetVerticesDirty();
		SetLayoutDirty();
	}

	virtual ETextAnchor GetAlignment() const override
	{
		return Alignment;
	}

	virtual void SetAlignment(ETextAnchor InAlignment) override
	{
		if (Alignment == InAlignment)
			return;
		
		Alignment = InAlignment;
		SetVerticesDirty();
		SetLayoutDirty();
	}

	virtual int32 GetFontSize() const override
	{
		return FontSize;
	}

	virtual void SetFontSize(int32 InFontSize) override
	{
		InFontSize = FMath::Clamp(InFontSize, 1, 300);
		if (FontSize == InFontSize)
			return;
		
		FontSize = InFontSize;
		SetVerticesDirty();
		SetLayoutDirty();
	}

	virtual EHorizontalWrapMode GetHorizontalOverflow() const override
	{
		return HorizontalOverflow;
	}

	virtual void SetHorizontalOverflow(EHorizontalWrapMode InHorizontalOverflow) override
	{
		if (HorizontalOverflow == InHorizontalOverflow)
			return;
		
		HorizontalOverflow = InHorizontalOverflow;
		SetVerticesDirty();
		SetLayoutDirty();
	}

	virtual EVerticalWrapMode GetVerticalOverflow() const override
	{
		return VerticalOverflow;
	}

	virtual void SetVerticalOverflow(EVerticalWrapMode InVerticalOverflow) override
	{
		if (VerticalOverflow == InVerticalOverflow)
			return;
		
		VerticalOverflow = InVerticalOverflow;
		SetVerticesDirty();
		SetLayoutDirty();
	}

	virtual float GetLineSpacing() const override
	{
		return LineSpacing;
	}

	virtual void SetLineSpacing(float InLineSpacing) override
	{
		if (LineSpacing == InLineSpacing)
			return;
		
		LineSpacing = InLineSpacing;
		SetVerticesDirty();
		SetLayoutDirty();
	}

	virtual float GetKerning() const override
	{
		return Kerning;
	}

	virtual void SetKerning(float InKerning) override
	{
		if (Kerning == InKerning)
			return;

		Kerning = InKerning;
		SetVerticesDirty();
		SetLayoutDirty();
	}

	virtual	float GetNonTextScale() const override
	{
		return NonTextScale;
	}
	
	virtual void SetNonTextScale(float InNonTextScale) override
	{
		if (NonTextScale == InNonTextScale)
			return;
		
		NonTextScale = InNonTextScale;
		SetVerticesDirty();
		SetLayoutDirty();
	}

	virtual	float GetUnderlineScale() const override
	{
		return UnderlineScale;
	}
	
	virtual void SetUnderlineScale(float InUnderlineScale) override
	{
		if (UnderlineScale == InUnderlineScale)
			return;
		
		UnderlineScale = InUnderlineScale;
		SetVerticesDirty();
	}

	virtual EFontStyle GetFontStyle() const override
	{
		return FontStyle;
	}

	virtual void SetFontStyle(EFontStyle InFontStyle) override
	{
		if (FontStyle == InFontStyle)
			return;
		
		FontStyle = InFontStyle;
		SetVerticesDirty();
		SetLayoutDirty();
	}
	
	virtual	bool IsBlendComponentColor() const override
	{
		return bBlendComponentColor;
	}
	
	virtual void SetBlendComponentColor(bool bNewBlendComponentColor) override
	{
		if (bNewBlendComponentColor != bBlendComponentColor)
		{
			bBlendComponentColor = bNewBlendComponentColor;
			SetVerticesDirty();
		}
	}

	virtual URectTransformComponent* GetTextTransformComponent() override
	{
		return AttachTransform;
	}

	virtual FTextWidget GetTextWidgetByName(FName InWidgetSymbol)  override
	{
		const auto TextWidgetPtr = TextWidgets.Find(InWidgetSymbol);
		if (TextWidgetPtr)
		{
			return *TextWidgetPtr;
		}
		return FTextWidget();
	}

	virtual void SetTextWidgetByName(FName InWidgetSymbol, FTextWidget InTextWidget) override
	{
		TextWidgets.Emplace(InWidgetSymbol, InTextWidget);
	}
	
public:
	//~ Begin BehaviourSubComponent Interface
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnDestroy() override;
	//~ End BehaviourSubComponent Interface.
	
protected:
	//~ Begin GraphicSubComponent Interface
	virtual void UpdateMaterial() override;
	virtual void UpdateGeometry() override;
	//~ End GraphicSubComponent Interface.
	
public:
	virtual FOnClickHyperlinkEvent& GetOnClickHyperlinkEvent() override
	{
		return OnHyperlinkClick;
	}
	
	virtual FTextGenerator& GetCachedTextGenerator() override
	{
		if (!TextCache.IsValid())
		{
			TextCache = MakeShareable(new FTextGenerator());
			TextCache->bGenerateCharacterAndLineInfo = bGenerateCharacterAndLineInfo;
		}
		return *TextCache.Get();
	}

	virtual void SetGenerateCharacterAndLineInfo(bool bInGenerateCharacterAndLineInfo) override
	{
		bGenerateCharacterAndLineInfo = bInGenerateCharacterAndLineInfo;
	}
	
	virtual FTextGenerationSettings GetGenerationSettings(FVector2D Extents) override;

protected:
	virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override;

public:
	//~ Begin ILayoutElementInterface Interface
	virtual void CalculateLayoutInputHorizontal() override {};
	virtual void CalculateLayoutInputVertical() override {};
	virtual float GetMinWidth() override { return 0; };
	virtual float GetPreferredWidth() override;
	virtual float GetFlexibleWidth() override { return -1; };
	virtual float GetMinHeight() override { return 0; };
	virtual float GetPreferredHeight() override;
	virtual float GetFlexibleHeight() override { return -1; };
	virtual int32 GetLayoutPriority() override { return 0; };
	//~ End ILayoutElementInterface Interface
	
public:
	virtual FTextEmoji* GetTextEmoji(FName InEmojiSymbol) override;
	virtual FTextWidget* GetTextWidget(FName InWidgetSymbol) override;
	virtual FAttachWidget* GetAttachWidget(FName InWidgetSymbol) override;

protected:
	UTextEmojiImageComponent* GenerateEmojiImageComponent() const;
	
	void ClearEmojis();
	void ResetEmojis();
	
	void TickEmojiAnimation();
	void UpdateEmojis(bool bOnlyUpdateAnimation, float DeltaTime = 0);

	UTextEmojiImageComponent* GetValidEmojiImageComponent(UTexture* InTexture);

	void OnAttachWidgetRectTransformDimensionsChange();
	
};
