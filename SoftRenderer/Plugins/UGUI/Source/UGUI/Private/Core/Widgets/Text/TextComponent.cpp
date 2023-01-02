#include "Core/Widgets/Text/TextComponent.h"
#include "UGUISettings.h"
#include "Core/MathUtility.h"
#include "Core/Layout/RectTransformUtility.h"
#include "Core/Render/VertexHelper.h"
#include "Core/Widgets/Text/TextGenerator.h"
#include "EventSystem/EventData/PointerEventData.h"
#include "UObject/ConstructorHelpers.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// UTextComponent

UTextComponent::UTextComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    bSupportRichText = true;
    bResizeTextForBestFit = false;
	bIgnoreTimeScaleForEmoji = false;
	bElipsizeEnd = false;
	bBlendComponentColor = false;
	bGenerateCharacterAndLineInfo = false;
	bNeedSetTimer = false;
	
    ResizeTextMinSize = 10;
    ResizeTextMaxSize = 40;

    Alignment = ETextAnchor::TextAnchor_UpperLeft;

    FontSize = 14;

    HorizontalOverflow = EHorizontalWrapMode::HorizontalWrapMode_Wrap;
    VerticalOverflow = EVerticalWrapMode::VerticalWrapMode_Truncate;

    LineSpacing = 1;
    Kerning = 0;
	NonTextScale = 1;
	UnderlineScale = 1.0;
	
    FontStyle = EFontStyle::FontStyle_Normal;
	
	static ConstructorHelpers::FObjectFinderOptional<USDFFont> DefaultTextFont_Finder(TEXT("SDFFont'/UGUI/DefaultResources/Font/UGUI_Default_Font.UGUI_Default_Font'"));
	Font = DefaultTextFont_Finder.Get();
}

void UTextComponent::SetText(FText InText)
{
	if (InText.IsEmpty())
	{
		if (Text.IsEmpty())
			return;

		ClearEmojis();
		Text = FText::GetEmpty();
		SetVerticesDirty();
	}
	else if (!Text.EqualTo(InText))
	{
		ClearEmojis();
		Text = InText;
		SetVerticesDirty();
		SetLayoutDirty();
	}
}

void UTextComponent::Awake()
{
	Super::Awake();

	URectTransformComponent* Widget = nullptr;
	for (auto& Elem : AttachWidgets)
	{
		Widget = Cast<URectTransformComponent>(GetChildComponent(Elem.Value.ChildWidgetIndex));
		if (IsValid(Widget))
		{
			Elem.Value.AttachWidget = Widget;
			Widget->SetLocalRotation(FRotator::ZeroRotator);
			Widget->SetLocalScale(FVector::OneVector);
			Widget->SetAnchor(FVector2D(0.5, 0.5), FVector2D(0.5, 0.5));
		//	FDelegateHandle AttachWidgetRectChangeDelegateHandle = Widget->OnRectTransformDimensionsChangeEvent.AddUObject(this, &UTextComponent::OnAttachWidgetRectTransformDimensionsChange);
		//	AttachWidgetRectChangeDelegateHandles.Add(Elem.Key, AttachWidgetRectChangeDelegateHandle);
		}
	}
	
	const auto World = GetWorld();
	if (IsValid(World))
	{
		LastTimeSeconds = bIgnoreTimeScaleForEmoji ? World->GetUnpausedTimeSeconds() :  World->GetTimeSeconds();	
	}
}

void UTextComponent::OnEnable()
{
    Super::OnEnable();

	if (TextCache.IsValid())
	{
        TextCache->Invalidate();
	}
}

void UTextComponent::OnDisable()
{
    TextCache.Reset();
    TextCacheForLayout.Reset();

	bNeedSetTimer = false;
	
	const auto World = GetWorld();
	if (IsValid(World))
	{
		if (TimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
			TimerHandle.Invalidate();
		}
	}
	
    Super::OnDisable();
}

void UTextComponent::OnDestroy()
{
	for (const auto& Elem : UsedEmojiImages)
	{
		if (Elem.Value.IsValid())
		{
			Elem.Value->DestroyComponent();
		}
	}
	
	for (const auto& EmojiImage : EmojiImages)
	{
		if (IsValid(EmojiImage))
		{
			EmojiImage->DestroyComponent();
		}
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (DebugEmojiImage.IsValid())
	{
		DebugEmojiImage->DestroyComponent();
	}
	DebugEmojiImage.Reset();
#endif

	for (auto& Elem : AttachWidgets)
	{
		if (IsValid(Elem.Value.AttachWidget) && AttachWidgetRectChangeDelegateHandles.Contains(Elem.Key))
		{
		//	Elem.Value.AttachWidget->OnRectTransformDimensionsChangeEvent.Remove(AttachWidgetRectChangeDelegateHandles[Elem.Key]);
		}
	}
	
	UsedEmojiImages.Empty();
	EmojiImages.Empty();
	CurEmojiList.Empty();
	HypertextRegionList.Empty();
	AttachWidgetRectChangeDelegateHandles.Empty();
	
	Super::OnDestroy();
}

void UTextComponent::UpdateMaterial()
{
	const auto CanvasRendererComp = GetCanvasRenderer();
	if (IsValid(CanvasRendererComp))
	{
		CanvasRendererComp->SetTextElement(true);
	}

	for (auto& EmojiImageElem : UsedEmojiImages)
	{
		if (EmojiImageElem.Value.IsValid())
		{
			EmojiImageElem.Value->SetMaskable(IsMaskable());
		}
	}

	Super::UpdateMaterial();
}

void UTextComponent::UpdateGeometry()
{
	if (IsValid(Font))
	{
        Super::UpdateGeometry();
	}
}

FTextGenerationSettings UTextComponent::GetGenerationSettings(FVector2D Extents)
{
    FTextGenerationSettings Settings;
    Settings.GenerationExtents = Extents;
    Settings.TextAnchor = Alignment;
    Settings.Color = Color;
    Settings.Font = Font;
    Settings.TextComponent = this;
    Settings.Pivot = Pivot;
    Settings.bRichText = bSupportRichText;
    Settings.LineSpacing = LineSpacing;
    Settings.Kerning = Kerning;
	Settings.NonTextScale = NonTextScale;
	Settings.UnderlineScale = UnderlineScale;
    Settings.FontStyle = FontStyle;
    Settings.bResizeTextForBestFit = bResizeTextForBestFit;
    Settings.HorizontalOverflow = HorizontalOverflow;
    Settings.VerticalOverflow = VerticalOverflow;
    Settings.FontSize = FontSize;
    Settings.ResizeTextMinSize = ResizeTextMinSize;
    Settings.ResizeTextMaxSize = ResizeTextMaxSize;
	Settings.bElipsizeEnd = bElipsizeEnd;
	Settings.bBlendComponentColor = bBlendComponentColor;

	if (IsValid(Font))
	{
		Settings.ImportFontSize = Font->ImportFontSize;
	}
	
    return MoveTemp(Settings);
}

DECLARE_CYCLE_STAT(TEXT("UIGraphic --- Text::OnPopulateMesh"), STAT_UnrealGUI_TextOnPopulateMesh, STATGROUP_UnrealGUI);
void UTextComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_TextOnPopulateMesh);
	
    if (!IsValid(Font))
    {
    	ClearEmojis();
    	HypertextRegionList.Empty();
	    return;
    }

	ResetEmojis();
 
    auto& CachedTextGenerator = GetCachedTextGenerator();
    CachedTextGenerator.Populate(Text.ToString(), GetGenerationSettings(GetRect().GetSize()), this);

    HypertextRegionList = MoveTemp(CachedTextGenerator.HypertextList);
    auto EmojiRegionList = MoveTemp(CachedTextGenerator.EmojiList);
    auto WidgetRegionList = MoveTemp(CachedTextGenerator.WidgetList);
	auto AttachWidgetRegionList = MoveTemp(CachedTextGenerator.AttachWidgetList);
    
    // Apply the offset to the vertices
    const auto& Vertices = CachedTextGenerator.Vertices;
    const int32 VertexCount = Vertices.Num();

    FVector2D RoundingOffset = FVector2D::ZeroVector;
	if (Vertices.Num() > 0)
	{
		RoundingOffset = FVector2D(Vertices[0].Position.X, Vertices[0].Position.Y);
	}
	else if (EmojiRegionList.Num() > 0)
	{
		RoundingOffset = FVector2D(EmojiRegionList[0].BottomLeft.X, EmojiRegionList[0].BottomLeft.Y);
	}
	else if (WidgetRegionList.Num() > 0)
	{
		RoundingOffset = FVector2D(WidgetRegionList[0].BottomLeft.X, WidgetRegionList[0].BottomLeft.Y);
	}
	else if (AttachWidgetRegionList.Num() > 0)
	{
		RoundingOffset = FVector2D(AttachWidgetRegionList[0].BottomLeft.X, AttachWidgetRegionList[0].BottomLeft.Y);
	}
	
	RoundingOffset = PixelAdjustPoint(RoundingOffset) - RoundingOffset;
    VertexHelper.Reset();

    const FVector2D UV1 = GetUV1FromGraphicEffects();
	
    if (!(FMathUtility::Approximately(RoundingOffset.X, 0.0f) && FMathUtility::Approximately(RoundingOffset.Y, 0.0f)))
    {
        TempUIVertices.SetNum(4);
        for (int32 Index = 0; Index < VertexCount; ++Index)
        {
            const int32 TempVertexIndex = Index & 3;

            TempUIVertices[TempVertexIndex] = Vertices[Index];
            TempUIVertices[TempVertexIndex].Position.X += RoundingOffset.X;
            TempUIVertices[TempVertexIndex].Position.Y += RoundingOffset.Y;
            TempUIVertices[TempVertexIndex].UV1 = UV1;
        	
        	if (TempVertexIndex == 3)
            {
                VertexHelper.AddUIVertexQuad(TempUIVertices);
            }
        }

        for (int32 Index = 0, Count = HypertextRegionList.Num(); Index < Count; ++Index)
        {
            HypertextRegionList[Index].BottomLeft.X += RoundingOffset.X;
            HypertextRegionList[Index].BottomLeft.Y += RoundingOffset.Y;
            
            HypertextRegionList[Index].TopRight.X += RoundingOffset.X;
            HypertextRegionList[Index].TopRight.Y += RoundingOffset.Y;
        }

        for (int32 Index = 0, Count = EmojiRegionList.Num(); Index < Count; ++Index)
        {
            EmojiRegionList[Index].BottomLeft.X += RoundingOffset.X;
            EmojiRegionList[Index].BottomLeft.Y += RoundingOffset.Y;
            
            EmojiRegionList[Index].TopRight.X += RoundingOffset.X;
            EmojiRegionList[Index].TopRight.Y += RoundingOffset.Y;
        }

        for (int32 Index = 0, Count = WidgetRegionList.Num(); Index < Count; ++Index)
        {
            WidgetRegionList[Index].BottomLeft.X += RoundingOffset.X;
            WidgetRegionList[Index].BottomLeft.Y += RoundingOffset.Y;
            
            WidgetRegionList[Index].TopRight.X += RoundingOffset.X;
            WidgetRegionList[Index].TopRight.Y += RoundingOffset.Y;
        }

    	for (int32 Index = 0, Count = AttachWidgetRegionList.Num(); Index < Count; ++Index)
    	{
    		AttachWidgetRegionList[Index].BottomLeft.X += RoundingOffset.X;
    		AttachWidgetRegionList[Index].BottomLeft.Y += RoundingOffset.Y;
            
    		AttachWidgetRegionList[Index].TopRight.X += RoundingOffset.X;
    		AttachWidgetRegionList[Index].TopRight.Y += RoundingOffset.Y;
    	}
    }
    else
    {
        TempUIVertices.SetNum(4);
        for (int32 Index = 0; Index < VertexCount; ++Index)
        {
            const int32 TempVertexIndex = Index & 3;

            TempUIVertices[TempVertexIndex] = Vertices[Index];
            TempUIVertices[TempVertexIndex].UV1 = UV1;
        	
            if (TempVertexIndex == 3)
            {
                VertexHelper.AddUIVertexQuad(TempUIVertices);
            }
        }
    }
	
	bool bAnimation = false;
	
	CurEmojiList.SetNum(EmojiRegionList.Num());
	for (int32 Index = 0, Count = EmojiRegionList.Num(); Index < Count; ++Index)
	{
		auto& EmojiInfo = CurEmojiList[Index];
		EmojiInfo.EmojiImageComponent = nullptr;
		
		auto& EmojiRegionInfo = EmojiRegionList[Index];
		EmojiInfo.EmojiRegion = EmojiRegionInfo;
		
		FTextEmoji* TextEmoji = GetTextEmoji(EmojiRegionInfo.EmojiSymbol);
		if (TextEmoji)
		{
			if (!bAnimation && TextEmoji->bAnimation)
			{
				bAnimation = true;
			}
		}
		else
		{
			EmojiInfo.EmojiRegion.bVisible = false;
		}
	}
	
	for (auto& WidgetRegion : WidgetRegionList)
	{
		FTextWidget* Widget = GetTextWidget(WidgetRegion.WidgetSymbol);
		if (IsValid(Widget->WidgetRectTransform))
		{
			Widget->WidgetRectTransform->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
			// TODO 优化成一个函数
			Widget->WidgetRectTransform->SetLocalRotation(FRotator::ZeroRotator);
			Widget->WidgetRectTransform->SetLocalScale(FVector::OneVector);
			Widget->WidgetRectTransform->SetSizeDelta(FVector2D(WidgetRegion.TopRight.X - WidgetRegion.BottomLeft.X,WidgetRegion.TopRight.Y - WidgetRegion.BottomLeft.Y));
			Widget->WidgetRectTransform->SetAnchorAndPosition(FVector2D(0.5, 0.5),
				FVector2D(0.5, 0.5), FVector2D((WidgetRegion.BottomLeft.X + WidgetRegion.TopRight.X) * 0.5f,
					(WidgetRegion.BottomLeft.Y + WidgetRegion.TopRight.Y) * 0.5f));
		}
	}

	for (auto& AttachWidgetRegion : AttachWidgetRegionList)
	{
		FAttachWidget* Widget = GetAttachWidget(AttachWidgetRegion.WidgetSymbol);
		if (IsValid(Widget->AttachWidget))
		{
			Widget->AttachWidget->SetAnchoredPosition(FVector2D((AttachWidgetRegion.BottomLeft.X + AttachWidgetRegion.TopRight.X) * 0.5f,
					(AttachWidgetRegion.BottomLeft.Y + AttachWidgetRegion.TopRight.Y) * 0.5f));
		}
	}

	const auto World = GetWorld();
	if (IsValid(World))
	{
		if (bAnimation)
		{
			bNeedSetTimer = true;
			
			if (!TimerHandle.IsValid())
			{
				World->GetTimerManager().SetTimer(TimerHandle,this, &UTextComponent::TickEmojiAnimation, 0.001f, false);
			}
		}
		else
		{
			bNeedSetTimer = false;
			
			if (TimerHandle.IsValid())
			{
				World->GetTimerManager().ClearTimer(TimerHandle);
				TimerHandle.Invalidate();
			}
		}
	}

	UpdateEmojis(false);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (UUGUISettings::Get()->bShowRichTextDebugRegion)
	{
		if (!DebugEmojiImage.IsValid())
		{
			DebugEmojiImage = GenerateEmojiImageComponent();
		}
		else
		{
			DebugEmojiImage->ClearEmojiRenderInfos();
		}
		
		for (auto& HypertextRegion : HypertextRegionList)
		{
			FEmojiRenderInfo EmojiRenderClip = FEmojiRenderInfo();
			EmojiRenderClip.BottomLeft = HypertextRegion.BottomLeft;
			EmojiRenderClip.TopRight = HypertextRegion.TopRight;
			EmojiRenderClip.BottomLeftUV = FVector2D(0, 1);
			EmojiRenderClip.TopRightUV = FVector2D(1, 0);
			EmojiRenderClip.EmojiColor = FLinearColor(1, 0, 0, 0.5);
			
			if (DebugEmojiImage.IsValid())
			{
				DebugEmojiImage->AddEmojiRenderInfo(MoveTemp(EmojiRenderClip));
			}
		}

		for (auto& EmojiRegion : EmojiRegionList)
		{
			FEmojiRenderInfo EmojiRenderClip = FEmojiRenderInfo();
			EmojiRenderClip.BottomLeft = EmojiRegion.BottomLeft;
			EmojiRenderClip.TopRight = EmojiRegion.TopRight;
			EmojiRenderClip.BottomLeftUV = FVector2D(0, 1);
			EmojiRenderClip.TopRightUV = FVector2D(1, 0);
			EmojiRenderClip.EmojiColor = FLinearColor(1, 0, 0, 0.5);
			
			if (DebugEmojiImage.IsValid())
			{
				DebugEmojiImage->AddEmojiRenderInfo(MoveTemp(EmojiRenderClip));
			}
		}

		for (auto& WidgetRegion : WidgetRegionList)
		{
			FEmojiRenderInfo EmojiRenderClip = FEmojiRenderInfo();
			EmojiRenderClip.BottomLeft = WidgetRegion.BottomLeft;
			EmojiRenderClip.TopRight = WidgetRegion.TopRight;
			EmojiRenderClip.BottomLeftUV = FVector2D(0, 1);
			EmojiRenderClip.TopRightUV = FVector2D(1, 0);
			EmojiRenderClip.EmojiColor = FLinearColor(1, 0, 0, 0.5);
			
			if (DebugEmojiImage.IsValid())
			{
				DebugEmojiImage->AddEmojiRenderInfo(MoveTemp(EmojiRenderClip));
			}
		}

		for (auto& AttachWidgetRegion : AttachWidgetRegionList)
		{
			FEmojiRenderInfo AttachWidgetRenderClip = FEmojiRenderInfo();
			AttachWidgetRenderClip.BottomLeft = AttachWidgetRegion.BottomLeft;
			AttachWidgetRenderClip.TopRight = AttachWidgetRegion.TopRight;
			AttachWidgetRenderClip.BottomLeftUV = FVector2D(0, 1);
			AttachWidgetRenderClip.TopRightUV = FVector2D(1, 0);
			AttachWidgetRenderClip.EmojiColor = FLinearColor(1, 0, 0, 0.5);
			
			if (DebugEmojiImage.IsValid())
			{
				DebugEmojiImage->AddEmojiRenderInfo(MoveTemp(AttachWidgetRenderClip));
			}
		}

		if (DebugEmojiImage.IsValid())
		{
			DebugEmojiImage->SetVerticesDirty();
		}
	}
#endif
}

float UTextComponent::GetPreferredWidth()
{
    return GetCachedTextGeneratorForLayout().GetPreferredWidth(Text.ToString(), GetGenerationSettings(FVector2D::ZeroVector), this);
}

float UTextComponent::GetPreferredHeight()
{
    return GetCachedTextGeneratorForLayout().GetPreferredHeight(Text.ToString(), GetGenerationSettings(FVector2D(Rect.GetSize().X, 0)), this);
}

FTextEmoji* UTextComponent::GetTextEmoji(FName InEmojiSymbol)
{
	auto TextEmojiPtr = TextEmojis.Find(InEmojiSymbol);
	if (TextEmojiPtr)
	{
		return TextEmojiPtr;
	}

	for (const auto& EmojiSheet : TextEmojiSheets)
	{
		if (IsValid(EmojiSheet))
		{
			TextEmojiPtr = EmojiSheet->TextEmojis.Find(InEmojiSymbol);
			if (TextEmojiPtr)
			{
				return TextEmojiPtr;
			}
		}
	}
	
	return nullptr;
}

FTextWidget* UTextComponent::GetTextWidget(FName InWidgetSymbol)
{
    const auto TextWidgetPtr = TextWidgets.Find(InWidgetSymbol);
    if (TextWidgetPtr)
    {
        return TextWidgetPtr;
    }
    return nullptr;
}

FAttachWidget* UTextComponent::GetAttachWidget(FName InWidgetSymbol)
{
	const auto AttachWidgetPtr = AttachWidgets.Find(InWidgetSymbol);
	if (AttachWidgetPtr)
	{
		return AttachWidgetPtr;
	}
	return nullptr;
}

UTextEmojiImageComponent* UTextComponent::GenerateEmojiImageComponent()
{
	UTextEmojiImageComponent* EmojiImageComponent = NewObject<UTextEmojiImageComponent>(GetOwner(), UTextEmojiImageComponent::StaticClass(), NAME_None, RF_Transient);
	EmojiImageComponent->SetGraying(IsGrayingInHierarchy());
		
#if WITH_EDITOR
	EmojiImageComponent->bIsEditorOnly = true;

	if (UUGUISettings::Get()->bShowTextEmojiImageComponent)
	{
		EmojiImageComponent->CreationMethod = EComponentCreationMethod::Instance;
	}
#endif

	EmojiImageComponent->RegisterComponent();
	EmojiImageComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	EmojiImageComponent->SetLocalTransform(FTransform::Identity);

	EmojiImageComponent->SetAnchorAndOffsetAndPivot(FVector2D(0, 0), FVector2D(1, 1), FVector2D(0, 0), FVector2D(0, 0), GetPivot());
	EmojiImageComponent->AwakeFromLoad();
	return EmojiImageComponent;
}

void UTextComponent::ClearEmojis()
{
	CurEmojiList.Empty();

	ResetEmojis();

	bNeedSetTimer = false;
	
	const auto World = GetWorld();
	if (IsValid(World))
	{
		if (TimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
			TimerHandle.Invalidate();
		}
	}
}

void UTextComponent::ResetEmojis()
{
	for (const auto& Elem : UsedEmojiImages)
	{
		if (Elem.Value.IsValid())
		{
			Elem.Value->ClearEmojiRenderInfos();
			Elem.Value->SetVerticesDirty();
			
			EmojiImages.Add(Elem.Value.Get());
		}
	}

	UsedEmojiImages.Empty();

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (DebugEmojiImage.IsValid())
	{
		DebugEmojiImage->ClearEmojiRenderInfos();
		DebugEmojiImage->SetVerticesDirty();
	}
#endif
}

void UTextComponent::TickEmojiAnimation()
{
	const auto World = GetWorld();
	if (IsValid(World))
	{
		const float CurrentTimeSeconds = bIgnoreTimeScaleForEmoji ? World->GetUnpausedTimeSeconds() : World->GetTimeSeconds();
		const float DeltaTime = CurrentTimeSeconds - LastTimeSeconds;
		UpdateEmojis(true, DeltaTime);
		LastTimeSeconds = CurrentTimeSeconds;

		if (bNeedSetTimer)
		{
			World->GetTimerManager().SetTimer(TimerHandle,this, &UTextComponent::TickEmojiAnimation, 0.001f, false);
		}
	}
}

void UTextComponent::UpdateEmojis(bool bOnlyUpdateAnimation, float DeltaTime)
{
	for (auto& EmojiInfo : CurEmojiList)
	{
		FTextEmoji* TextEmoji = GetTextEmoji(EmojiInfo.EmojiRegion.EmojiSymbol);
		if (TextEmoji)
		{
			if (!bOnlyUpdateAnimation || TextEmoji->bAnimation)
			{
				if (TextEmoji->bAnimation)
				{
					bool bUpdateEmojiImage = false;
					
					const float Delta = EmojiInfo.AnimationDelta + DeltaTime;
					const float Rate = 1.0f / TextEmoji->FrameRate;
					if (Delta > Rate)
					{
						EmojiInfo.AnimationDelta = 0;

						++EmojiInfo.AnimationIndex;
						if (EmojiInfo.AnimationIndex >= TextEmoji->Sprites.Num())
						{
							EmojiInfo.AnimationIndex = 0;
						}

						bUpdateEmojiImage = true;
					}
					else
					{
						EmojiInfo.AnimationDelta = Delta;
					}

					if (bUpdateEmojiImage || !bOnlyUpdateAnimation)
					{
						if (EmojiInfo.EmojiRegion.bVisible)
						{
							const auto EmojiTexture = EmojiInfo.GetEmojiTexture(TextEmoji);
							if (!EmojiInfo.EmojiImageComponent.IsValid())
							{
								EmojiInfo.EmojiImageComponent = GetValidEmojiImageComponent(EmojiTexture);
							}
							else
							{
								if (EmojiInfo.EmojiImageComponent->GetMainTexture() != EmojiTexture)
								{
									EmojiInfo.RemoveEmojiRenderInfo();
									EmojiInfo.EmojiImageComponent = GetValidEmojiImageComponent(EmojiTexture);
								}
							}
						
							EmojiInfo.UpdateEmojiRenderInfo(TextEmoji, bBlendComponentColor ? Color : FLinearColor::White);
						}
					}

					if (!EmojiInfo.EmojiRegion.bVisible)
					{
						EmojiInfo.RemoveEmojiRenderInfo();
					}
				}
				else
				{
					if (EmojiInfo.EmojiRegion.bVisible)
					{
						const auto EmojiTexture = EmojiInfo.GetEmojiTexture(TextEmoji);
						if (!EmojiInfo.EmojiImageComponent.IsValid())
						{
							EmojiInfo.EmojiImageComponent = GetValidEmojiImageComponent(EmojiTexture);
						}
						else
						{
							if (EmojiInfo.EmojiImageComponent->GetMainTexture() != EmojiTexture)
							{
								EmojiInfo.RemoveEmojiRenderInfo();
								EmojiInfo.EmojiImageComponent = GetValidEmojiImageComponent(EmojiTexture);
							}
						}
						
						EmojiInfo.UpdateEmojiRenderInfo(TextEmoji, bBlendComponentColor ? Color : FLinearColor::White);
					}
					else
					{
						EmojiInfo.RemoveEmojiRenderInfo();
					}
				}				
			}
		}
	}
}

UTextEmojiImageComponent* UTextComponent::GetValidEmojiImageComponent(UTexture* InTexture)
{
	if (!IsValid(InTexture))
	{
		return nullptr;
	}

	const auto EmojiComponentPtr = UsedEmojiImages.Find(InTexture);
	if (!EmojiComponentPtr)
	{
		UTextEmojiImageComponent* NewEmojiImageComponent;
		if (EmojiImages.Num() > 0)
		{
			NewEmojiImageComponent = EmojiImages[EmojiImages.Num() - 1];
			EmojiImages.RemoveAt(EmojiImages.Num() - 1, 1, false);
		}
		else
		{
			NewEmojiImageComponent = GenerateEmojiImageComponent();
		}

		if (IsValid(NewEmojiImageComponent))
		{
			NewEmojiImageComponent->SetMaskable(IsMaskable());
			NewEmojiImageComponent->ClearEmojiRenderInfos();
			NewEmojiImageComponent->SetMainTexture(InTexture);
			UsedEmojiImages.Add(InTexture, NewEmojiImageComponent);
			return NewEmojiImageComponent;
		}

		return nullptr;
	}
	
	return EmojiComponentPtr->Get();
}

void UTextComponent::OnAttachWidgetRectTransformDimensionsChange()
{
	GetCachedTextGenerator().Invalidate();
	SetVerticesDirty();
	SetLayoutDirty();
}

/////////////////////////////////////////////////////
