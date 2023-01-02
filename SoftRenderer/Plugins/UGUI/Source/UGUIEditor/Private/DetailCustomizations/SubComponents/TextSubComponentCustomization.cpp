#include "DetailCustomizations/SubComponents/TextSubComponentCustomization.h"
#include "Core/Widgets/Text/TextSubComponent.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "UGUIEditorStyle.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FTextSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FTextSubComponentCustomization);
}

void FTextSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UTextSubComponent>(SubComponent.Get());

	AddRowHeaderContent(CategoryBuilder, DetailBuilder);
	AddProperty(TEXT("Text"),CategoryBuilder);
	
	{
		CategoryBuilder.AddCustomRow(LOCTEXT("TextSubComponent_CharacterRow", ""))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFontBold())
			.Text(LOCTEXT("TextSubComponent_Character", "Character"))
		]
		.ValueContent()
		[
			SNew(SSpacer)
		];

		AddProperty(TEXT("Font"), CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_Font", "\tFont"));
		AddProperty(TEXT("FontStyle"), CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_FontStyle", "\tFont Style"));
		AddProperty(TEXT("FontSize"),CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_FontSize", "\tFont Size"));
		AddProperty(TEXT("LineSpacing"),CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_LineSpacing", "\tLine  Spacing"));
		AddProperty(TEXT("Kerning"),CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_Kerning", "\tKerning"));
		
		const auto SupportRichTextRectProperty = AddProperty(TEXT("bSupportRichText"), CategoryBuilder);
		SupportRichTextRectProperty->DisplayName(LOCTEXT("TextSubComponent_SupportRichText", "\tRich Text"));
		SupportRichTextRectProperty->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
		
		AddProperty(TEXT("bElipsizeEnd"),CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_ElipsizeEnde", "\tElipsize End"));
	}

	{
		CategoryBuilder.AddCustomRow(LOCTEXT("TextSubComponent_ParagraphRow", ""))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFontBold())
			.Text(LOCTEXT("TextSubComponent_Paragraph", "Paragraph"))
		]
		.ValueContent()
		[
			SNew(SSpacer)
		];

		const auto AlignmentProperty = AddProperty(TEXT("Alignment"), CategoryBuilder);
		AlignmentProperty->Visibility(EVisibility::Collapsed);
		auto AlignmentHandle = AlignmentProperty->GetPropertyHandle();
		
		const TAttribute<ECheckBoxState> TextAlignmentLeft = TAttribute<ECheckBoxState>::Create([&]
		{
			if (TargetScriptPtr.IsValid())
			{
				const int32 TextAlignment = static_cast<int32>(TargetScriptPtr->GetAlignment());
				if (TextAlignment % 3 == 0)
				{
					return ECheckBoxState::Checked;
				}
			}
			return ECheckBoxState::Unchecked;
		});

		const auto HandleTextAlignmentLeft = FOnCheckStateChanged::CreateLambda([&, AlignmentHandle](ECheckBoxState CheckState)
		{
			if (CheckState == ECheckBoxState::Checked && TargetScriptPtr.IsValid())
			{
				const int32 TextAlignment = static_cast<int32>(TargetScriptPtr->GetAlignment());
				ETextAnchor NewTextAlignment = ETextAnchor::TextAnchor_UpperLeft;
				if (TextAlignment / 3 == 0)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_UpperLeft;
				}
				else if (TextAlignment / 3 == 1)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_MiddleLeft;
				}
				else if (TextAlignment / 3 == 2)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_LowerLeft;
				}

				AlignmentHandle->SetValue(static_cast<uint8>(NewTextAlignment));
			}
		});

		const TAttribute<ECheckBoxState> TextAlignmentCenter = TAttribute<ECheckBoxState>::Create([&]
		{
			if (TargetScriptPtr.IsValid())
			{
				const int32 TextAlignment = static_cast<int32>(TargetScriptPtr->GetAlignment());
				if (TextAlignment % 3 == 1)
				{
					return ECheckBoxState::Checked;
				}
			}
			return ECheckBoxState::Unchecked;
		});

		const auto HandleTextAlignmentCenter = FOnCheckStateChanged::CreateLambda([&, AlignmentHandle](ECheckBoxState CheckState)
		{
			if (CheckState == ECheckBoxState::Checked && TargetScriptPtr.IsValid())
			{
				const int32 TextAlignment = static_cast<int32>(TargetScriptPtr->GetAlignment());
				ETextAnchor NewTextAlignment = ETextAnchor::TextAnchor_UpperCenter;
				if (TextAlignment / 3 == 0)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_UpperCenter;
				}
				else if (TextAlignment / 3 == 1)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_MiddleCenter;
				}
				else if (TextAlignment / 3 == 2)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_LowerCenter;
				}

				AlignmentHandle->SetValue(static_cast<uint8>(NewTextAlignment));
			}
		});

		const TAttribute<ECheckBoxState> TextAlignmentRight = TAttribute<ECheckBoxState>::Create([&]
		{
			if (TargetScriptPtr.IsValid())
			{
				const int32 TextAlignment = static_cast<int32>(TargetScriptPtr->GetAlignment());
				if (TextAlignment % 3 == 2)
				{
					return ECheckBoxState::Checked;
				}
			}
			return ECheckBoxState::Unchecked;
		});

		const auto HandleTextAlignmentRight = FOnCheckStateChanged::CreateLambda([&, AlignmentHandle](ECheckBoxState CheckState)
		{
			if (CheckState == ECheckBoxState::Checked && TargetScriptPtr.IsValid())
			{
				const int32 TextAlignment = static_cast<int32>(TargetScriptPtr->GetAlignment());
				ETextAnchor NewTextAlignment = ETextAnchor::TextAnchor_UpperCenter;
				if (TextAlignment / 3 == 0)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_UpperRight;
				}
				else if (TextAlignment / 3 == 1)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_MiddleRight;
				}
				else if (TextAlignment / 3 == 2)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_LowerRight;
				}

				AlignmentHandle->SetValue(static_cast<uint8>(NewTextAlignment));
			}
		});

		const TAttribute<ECheckBoxState> TextAlignmentUpper = TAttribute<ECheckBoxState>::Create([&]
		{
			if (TargetScriptPtr.IsValid())
			{
				const int32 TextAlignment = static_cast<int32>(TargetScriptPtr->GetAlignment());
				if (TextAlignment / 3 == 0)
				{
					return ECheckBoxState::Checked;
				}
			}
			return ECheckBoxState::Unchecked;
		});

		const auto HandleTextAlignmentUpper = FOnCheckStateChanged::CreateLambda([&, AlignmentHandle](ECheckBoxState CheckState)
		{
			if (CheckState == ECheckBoxState::Checked && TargetScriptPtr.IsValid())
			{
				const int32 TextAlignment = static_cast<int32>(TargetScriptPtr->GetAlignment());
				ETextAnchor NewTextAlignment = ETextAnchor::TextAnchor_UpperLeft;
				if (TextAlignment % 3 == 0)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_UpperLeft;
				}
				else if (TextAlignment % 3 == 1)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_UpperCenter;
				}
				else if (TextAlignment % 3 == 2)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_UpperRight;
				}

				AlignmentHandle->SetValue(static_cast<uint8>(NewTextAlignment));
			}
		});

		const TAttribute<ECheckBoxState> TextAlignmentMiddle = TAttribute<ECheckBoxState>::Create([&]
		{
			if (TargetScriptPtr.IsValid())
			{
				const int32 TextAlignment = static_cast<int32>(TargetScriptPtr->GetAlignment());
				if (TextAlignment / 3 == 1)
				{
					return ECheckBoxState::Checked;
				}
			}
			return ECheckBoxState::Unchecked;
		});

		const auto HandleTextAlignmentMiddle = FOnCheckStateChanged::CreateLambda([&, AlignmentHandle](ECheckBoxState CheckState)
		{
			if (CheckState == ECheckBoxState::Checked && TargetScriptPtr.IsValid())
			{
				const int32 TextAlignment = static_cast<int32>(TargetScriptPtr->GetAlignment());
				ETextAnchor NewTextAlignment = ETextAnchor::TextAnchor_MiddleLeft;
				if (TextAlignment % 3 == 0)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_MiddleLeft;
				}
				else if (TextAlignment % 3 == 1)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_MiddleCenter;
				}
				else if (TextAlignment % 3 == 2)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_MiddleRight;
				}

				AlignmentHandle->SetValue(static_cast<uint8>(NewTextAlignment));
			}
		});

		const TAttribute<ECheckBoxState> TextAlignmentLower = TAttribute<ECheckBoxState>::Create([&]
		{
			if (TargetScriptPtr.IsValid())
			{
				const int32 TextAlignment = static_cast<int32>(TargetScriptPtr->GetAlignment());
				if (TextAlignment / 3 == 2)
				{
					return ECheckBoxState::Checked;
				}
			}
			return ECheckBoxState::Unchecked;
		});

		const auto HandleTextAlignmentLower = FOnCheckStateChanged::CreateLambda([&, AlignmentHandle](ECheckBoxState CheckState)
		{
			if (CheckState == ECheckBoxState::Checked && TargetScriptPtr.IsValid())
			{
				const int32 TextAlignment = static_cast<int32>(TargetScriptPtr->GetAlignment());
				ETextAnchor NewTextAlignment = ETextAnchor::TextAnchor_LowerLeft;
				if (TextAlignment % 3 == 0)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_LowerLeft;
				}
				else if (TextAlignment % 3 == 1)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_LowerCenter;
				}
				else if (TextAlignment % 3 == 2)
				{
					NewTextAlignment = ETextAnchor::TextAnchor_LowerRight;
				}

				AlignmentHandle->SetValue(static_cast<uint8>(NewTextAlignment));
			}
		});
		
		CategoryBuilder.AddCustomRow(LOCTEXT("TextSubComponent_AlignmentRow", "Alignment"))
		.NameContent()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TextSubComponent_Alignment", "\tAlignment"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		]
		.ValueContent()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 1, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SNew(SCheckBox)
				.Cursor( EMouseCursor::Default )
				.Padding(FMargin( 4 ))
				.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
				.OnCheckStateChanged(HandleTextAlignmentLeft)
				.IsChecked(TextAlignmentLeft)
				.Content()
				[
					SNew( SBox )
					.WidthOverride( 12 )
					.HeightOverride( 12 )
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FUGUIEditorStyle::Get().GetBrush("TextIcon.HAlignLeft"))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 1, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SNew(SCheckBox)
				.Cursor( EMouseCursor::Default )
				.Padding(FMargin( 4 ))
				.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
				.OnCheckStateChanged(HandleTextAlignmentCenter)
				.IsChecked(TextAlignmentCenter)
				.Content()
				[
					SNew( SBox )
					.WidthOverride( 12 )
					.HeightOverride( 12 )
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FUGUIEditorStyle::Get().GetBrush("TextIcon.HAlignCenter"))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 0, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SNew(SCheckBox)
				.Cursor( EMouseCursor::Default )
				.Padding(FMargin( 4 ))
				.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
				.OnCheckStateChanged(HandleTextAlignmentRight)
				.IsChecked(TextAlignmentRight)
				.Content()
				[
					SNew( SBox )
					.WidthOverride( 12 )
					.HeightOverride( 12 )
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FUGUIEditorStyle::Get().GetBrush("TextIcon.HAlignRight"))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(16, 0, 0, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 1, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SNew(SCheckBox)
				.Cursor( EMouseCursor::Default )
				.Padding(FMargin( 4 ))
				.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
				.OnCheckStateChanged(HandleTextAlignmentUpper)
				.IsChecked(TextAlignmentUpper)
				.Content()
				[
					SNew( SBox )
					.WidthOverride( 12 )
					.HeightOverride( 12 )
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FUGUIEditorStyle::Get().GetBrush("TextIcon.VAlignTop"))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 1, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SNew(SCheckBox)
				.Cursor( EMouseCursor::Default )
				.Padding(FMargin( 4 ))
				.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
				.OnCheckStateChanged(HandleTextAlignmentMiddle)
				.IsChecked(TextAlignmentMiddle)
				.Content()
				[
					SNew( SBox )
					.WidthOverride( 12 )
					.HeightOverride( 12 )
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FUGUIEditorStyle::Get().GetBrush(("TextIcon.VAlignMiddle")))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 0, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SNew(SCheckBox)
				.Cursor( EMouseCursor::Default )
				.Padding(FMargin( 4 ))
				.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
				.OnCheckStateChanged(HandleTextAlignmentLower)
				.IsChecked(TextAlignmentLower)
				.Content()
				[
					SNew( SBox )
					.WidthOverride( 12 )
					.HeightOverride( 12 )
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FUGUIEditorStyle::Get().GetBrush(("TextIcon.VAlignBottom")))
					]
				]
			]
		];

		AddProperty(TEXT("HorizontalOverflow"),CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_HorizontalOverflow", "\tHorizontal Overflow"));
		AddProperty(TEXT("VerticalOverflow"), CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_VerticalOverflow", "\tVertical Overflow"));

		const auto BestFitProperty = AddProperty(TEXT("bResizeTextForBestFit"), CategoryBuilder);
		BestFitProperty->DisplayName(LOCTEXT("TextSubComponent_BestFit", "\tBest Fit"));
		BestFitProperty->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

		if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsResizeTextForBestFit())
		{
			AddProperty(TEXT("ResizeTextMinSize"), CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_MinSize", "\tMin Size"));
			AddProperty(TEXT("ResizeTextMaxSize"), CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_MaxSize", "\tMax Size"));
		}
	}
	
	if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsSupportRichText())
	{
		CategoryBuilder.AddCustomRow(LOCTEXT("TextSubComponent_RichTextRow", ""))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFontBold())
			.Text(LOCTEXT("TextSubComponent_RichText", "Rich Text"))
		]
		.ValueContent()
		[
			SNew(SSpacer)
		];
		
		AddProperty(TEXT("NonTextScale"),CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_NonTextScale", "\tNon Text Scale"));
		AddProperty(TEXT("UnderlineScale"), CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_UnderlineScale", "\tUnderline Scale"));
	
		AddProperty(TEXT("TextWidgets"), CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_NonTextScale", "\tText Widgets"));
		AddProperty(TEXT("AttachWidgets"), CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_AttachWidgets", "\tAttach Widgets"));
		AddProperty(TEXT("TextEmojis"), CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_TextEmojis", "\tText Emojis"));
		AddProperty(TEXT("TextEmojiSheets"), CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_TextEmojiSheets", "\tText Emoji Sheets"));
	
		AddProperty(TEXT("bIgnoreTimeScaleForEmoji"), CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_IgnoreTimeScaleForEmoji", "\tIgnore Time Scale For Emoji"));
		AddProperty(TEXT("bBlendComponentColor"), CategoryBuilder)->DisplayName(LOCTEXT("TextSubComponent_BlendComponentColor", "\tBlend Component Color"));
	}
	
	AddProperty(TEXT("Color"),CategoryBuilder);
	AddProperty(TEXT("Material"), CategoryBuilder);
	
	AddProperty(TEXT("bRaycastTarget"), CategoryBuilder);
	AddProperty(TEXT("bMaskable"), CategoryBuilder);

	AddProperty(TEXT("bGraying"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("bInvertColor"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("RenderOpacity"), CategoryBuilder, EPropertyLocation::Advanced);
}

#undef LOCTEXT_NAMESPACE
