#include "DetailCustomizations/TextComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "UGUIEditorStyle.h"
#include "Widgets/Text/STextBlock.h"
#include "Core/Widgets/GraphicComponent.h"
#include "Core/Widgets/MaskableGraphicComponent.h"
#include "Core/Widgets/Text/TextComponent.h"

#define LOCTEXT_NAMESPACE "UGUITextEditor"

TSharedRef<IDetailCustomization> FTextComponentDetails::MakeInstance()
{
	return MakeShareable(new FTextComponentDetails);
}

void FTextComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Appearance"));
	DetailBuilder.HideCategory(TEXT("Events"));
	DetailBuilder.HideCategory(TEXT("Content"));
	DetailBuilder.HideCategory(TEXT("Graphic"));

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UTextComponent>(TargetObjects[0].Get());
	
	IDetailCategoryBuilder& TextCategory = DetailBuilder.EditCategory(TEXT("Text"),FText(LOCTEXT("TextComponentCategory","Text")),ECategoryPriority::Important);

	AddRowHeaderContent(TextCategory, DetailBuilder);
	
	TextCategory.AddProperty(TEXT("Text"));

	{
		TextCategory.AddCustomRow(LOCTEXT("TextSubComponent_CharacterRow", ""))
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

		TextCategory.AddProperty(TEXT("Font")).DisplayName(LOCTEXT("TextSubComponent_Font", "\tFont"));
		TextCategory.AddProperty(TEXT("FontStyle")).DisplayName(LOCTEXT("TextSubComponent_FontStyle", "\tFont Style"));
		TextCategory.AddProperty(TEXT("FontSize")).DisplayName(LOCTEXT("TextSubComponent_FontSize", "\tFont Size"));
		TextCategory.AddProperty(TEXT("LineSpacing")).DisplayName(LOCTEXT("TextSubComponent_LineSpacing", "\tLine  Spacing"));
		TextCategory.AddProperty(TEXT("Kerning")).DisplayName(LOCTEXT("TextSubComponent_Kerning", "\tKerning"));

		DetailBuilder.GetProperty(TEXT("bSupportRichText"))
				->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
		TextCategory.AddProperty(TEXT("bSupportRichText")).DisplayName(LOCTEXT("TextSubComponent_SupportRichText", "\tRich Text"));
		
		TextCategory.AddProperty(TEXT("bElipsizeEnd")).DisplayName(LOCTEXT("TextSubComponent_ElipsizeEnde", "\tElipsize End"));
	}
	
	{
		TextCategory.AddCustomRow(LOCTEXT("TextSubComponent_ParagraphRow", ""))
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
		
		auto AlignmentHandle = DetailBuilder.GetProperty(TEXT("Alignment"));

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
		
		TextCategory.AddCustomRow(LOCTEXT("TextComponent_AlignmentRow", "Alignment"))
		.NameContent()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TextComponent_Alignment", "\tAlignment"))
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

		TextCategory.AddProperty(TEXT("HorizontalOverflow")).DisplayName(LOCTEXT("TextSubComponent_HorizontalOverflow", "\tHorizontal Overflow"));
		TextCategory.AddProperty(TEXT("VerticalOverflow")).DisplayName(LOCTEXT("TextSubComponent_VerticalOverflow", "\tVertical Overflow"));
		
		DetailBuilder.GetProperty(TEXT("bResizeTextForBestFit"))
				->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
		TextCategory.AddProperty(TEXT("bResizeTextForBestFit")).DisplayName(LOCTEXT("TextSubComponent_BestFit", "\tBest Fit"));

		if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsResizeTextForBestFit())
		{
			TextCategory.AddProperty(TEXT("ResizeTextMinSize")).DisplayName(LOCTEXT("TextSubComponent_MinSize", "\tMin Size"));
			TextCategory.AddProperty(TEXT("ResizeTextMaxSize")).DisplayName(LOCTEXT("TextSubComponent_MaxSize", "\tMax Size"));
		}
	}

	if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsSupportRichText())
	{
		TextCategory.AddCustomRow(LOCTEXT("TextSubComponent_RichTextRow", ""))
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
		
		TextCategory.AddProperty(TEXT("NonTextScale")).DisplayName(LOCTEXT("TextSubComponent_NonTextScale", "\tNon Text Scale"));
		TextCategory.AddProperty(TEXT("UnderlineScale")).DisplayName(LOCTEXT("TextSubComponent_UnderlineScale", "\tUnderline Scale"));
	
		TextCategory.AddProperty(TEXT("TextWidgets")).DisplayName(LOCTEXT("TextSubComponent_NonTextScale", "\tText Widgets"));
		TextCategory.AddProperty(TEXT("AttachWidgets")).DisplayName(LOCTEXT("TextSubComponent_AttachWidgets", "\tAttach Widgets"));
		TextCategory.AddProperty(TEXT("TextEmojis")).DisplayName(LOCTEXT("TextSubComponent_TextEmojis", "\tText Emojis"));
		TextCategory.AddProperty(TEXT("TextEmojiSheets")).DisplayName(LOCTEXT("TextSubComponent_TextEmojiSheets", "\tText Emoji Sheets"));
	
		TextCategory.AddProperty(TEXT("bIgnoreTimeScaleForEmoji")).DisplayName(LOCTEXT("TextSubComponent_IgnoreTimeScaleForEmoji", "\tIgnore Time Scale For Emoji"));
		TextCategory.AddProperty(TEXT("bBlendComponentColor")).DisplayName(LOCTEXT("TextComponent_BlendComponentColor", "\tBlend Component Color"));
	}
	
	TextCategory.AddProperty(TEXT("Color"), UGraphicComponent::StaticClass());
	TextCategory.AddProperty(TEXT("Material"), UGraphicComponent::StaticClass());
	
	TextCategory.AddProperty(TEXT("bRaycastTarget"), UGraphicComponent::StaticClass());
	TextCategory.AddProperty(TEXT("bMaskable"), UMaskableGraphicComponent::StaticClass());

	AddEventProperty(TextCategory, DetailBuilder, TEXT("OnHyperlinkClick"));
}

#undef LOCTEXT_NAMESPACE
