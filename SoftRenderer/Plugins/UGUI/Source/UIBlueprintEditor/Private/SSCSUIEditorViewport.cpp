#include "SSCSUIEditorViewport.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBorder.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "SSCSComponentEditor.h"
#include "Slate/SceneViewport.h"
#include "SViewportToolBar.h"
#include "STransformViewportToolbar.h"
#include "EditorViewportCommands.h"
#include "SEditorViewportToolBarMenu.h"
#include "BlueprintEditorTabs.h"
#include "BlueprintEditorSettings.h"
#include "UIBlueprintEditorCommands.h"
#include "UGUISubsystem.h"
#include "Designer/UGUIDesignerWorldSubsystem.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/SlateTypes.h"
#include "Framework/Commands/UIAction.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Framework/MultiBox/MultiBoxDefs.h"
#include "Widgets/Input/SCheckBox.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorViewportClient.h"
#include "SEditorViewport.h"
#include "SViewportToolBarIconMenu.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Settings/EditorProjectSettings.h"
#include "LevelEditor.h"
#include "UGUISettings.h"
#include "UIBlueprintEditorStyle.h"
#include "UIEditorPerProjectUserSettings.h"

#define LOCTEXT_NAMESPACE "BlueprintEditor"

/*-----------------------------------------------------------------------------
   SSCSUIEditorViewportToolBar
-----------------------------------------------------------------------------*/

class SSCSUIEditorViewportToolBar : public SViewportToolBar
{
public:
	SLATE_BEGIN_ARGS( SSCSUIEditorViewportToolBar ){}
	    SLATE_EVENT(FOnCamSpeedChanged, OnCamSpeedChanged)
		SLATE_EVENT(FOnCamSpeedScalarChanged, OnCamSpeedScalarChanged)
		SLATE_ARGUMENT(TWeakPtr<SSCSUIEditorViewport>, EditorViewport)
	SLATE_END_ARGS()

	/** Constructs this widget with the given parameters */
	void Construct(const FArguments& InArgs)
	{
		EditorViewport = InArgs._EditorViewport;

		static const FName DefaultForegroundName("DefaultForeground");

		const FName StyleName = TEXT("ViewportMenu");
		const FName CheckboxStyle = FEditorStyle::Join(StyleName, ".ToggleButton");
		
		this->ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("NoBorder"))
			.ColorAndOpacity(this, &SViewportToolBar::OnGetColorAndOpacity)
			.ForegroundColor(FEditorStyle::GetSlateColor(DefaultForegroundName))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 2.0f)
				[
					SNew(SEditorViewportToolbarMenu)
					.ParentToolBar(SharedThis(this))
					.Cursor(EMouseCursor::Default)
					.Image("EditorViewportToolBar.MenuDropdown")
					.OnGetMenuContent(this, &SSCSUIEditorViewportToolBar::GeneratePreviewMenu)
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 2.0f)
				.VAlign(VAlign_Fill)
				[
					SNew(SCheckBox)
					.Cursor( EMouseCursor::Default )
					.Padding(FMargin( 4, 4 ))
					.Style(FEditorStyle::Get(), EMultiBlockLocation::ToName(CheckboxStyle, EMultiBlockLocation::Type::None))
					.OnCheckStateChanged(this, &SSCSUIEditorViewportToolBar::HandleToggleViewportViewMode)
					.ToolTipText(LOCTEXT("UIViewprotMode_ToolTip", "When toggled on, the Scene is in 2D view. When toggled off, the Scene is in 3D view"))
					.IsChecked(this, &SSCSUIEditorViewportToolBar::Is2DViewportViewModeChecked)
					.Content()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.Padding(0)
						.VAlign(VAlign_Center)
						[
							SNew( STextBlock )
							.Font( FEditorStyle::GetFontStyle("EditorViewportToolBar.Font") )
							.Text( this, &SSCSUIEditorViewportToolBar::GetViewportViewModeLabel )
							.TextStyle( FEditorStyle::Get(), FEditorStyle::Join( StyleName, ".Label" ) )
						]						
					]
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 2.0f)
				.VAlign(VAlign_Fill)
				[
					SNew(SCheckBox)
					.Cursor( EMouseCursor::Default )
					.Padding(FMargin( 4, 4 ))
					.Style(FEditorStyle::Get(), EMultiBlockLocation::ToName(CheckboxStyle, EMultiBlockLocation::Type::None))
					.OnCheckStateChanged(this, &SSCSUIEditorViewportToolBar::HandleToggleSimulateGameOverlay)
					.ToolTipText(LOCTEXT("UIViewprotOverlayMode_ToolTip", "Simulate the overlay mode of canvas"))
					.IsChecked(this, &SSCSUIEditorViewportToolBar::IsSimulateGameOverlayChecked)
					.Content()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.Padding(0)
						.VAlign(VAlign_Center)
						[
							SNew( STextBlock )
							.Font( FEditorStyle::GetFontStyle("EditorViewportToolBar.Font") )
							.Text( this, &SSCSUIEditorViewportToolBar::GetIsSimulateGameOverlayLabel )
							.TextStyle( FEditorStyle::Get(), FEditorStyle::Join( StyleName, ".Label" ) )
						]
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 2.0f)
				[
					SNew( SEditorViewportToolbarMenu )
					.ParentToolBar( SharedThis( this ) )
					.Cursor( EMouseCursor::Default )
					.Label(this, &SSCSUIEditorViewportToolBar::GetViewMenuLabel)
					.LabelIcon(this, &SSCSUIEditorViewportToolBar::GetViewMenuLabelIcon)
					.OnGetMenuContent(this, &SSCSUIEditorViewportToolBar::GenerateViewMenu)
				]
				+ SHorizontalBox::Slot()
				.Padding(2.0f, 2.0f)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				[
					SNew(SCheckBox)
					.Cursor( EMouseCursor::Default )
					.Padding(FMargin( 4.0f ))
					.Style(FEditorStyle::Get(), EMultiBlockLocation::ToName(TEXT("ViewportMenu.ToggleButton"), EMultiBlockLocation::Start))
					.OnCheckStateChanged(this, &SSCSUIEditorViewportToolBar::HandleToggleLocalizationPreview)
					.ToolTipText(NSLOCTEXT("DesignerCommands", "Toggle Localization Preview", "Enables or disables the localization preview for the current preview language (see Editor Settings -> Region & Language)."))
					.IsChecked(this, &SSCSUIEditorViewportToolBar::IsLocalizationPreviewChecked)
					.Content()
					[
						SNew( SBox )
						.WidthOverride( 16 )
						.HeightOverride( 16 )
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(GetLocalizationIcon())
						]
					]
				]
				/*+ SHorizontalBox::Slot()
				.Padding(2.0f, 2.0f)
				.AutoWidth()
				.VAlign(VAlign_Fill)
				[
					SNew(SButton)
					.ButtonStyle( FEditorStyle::Get(), EMultiBlockLocation::ToName(TEXT("ViewportMenu.ToggleButton"), EMultiBlockLocation::End) )
					.ContentPadding( FMargin( 5.0f, 0.0f ) )
					.ToolTipText(NSLOCTEXT("UMG", "ToggleLocalizationPreview_MenuToolTip", "Choose the localization preview language"))
					.OnClicked(this, &SSCSUIEditorViewportToolBar::OnLocalizationPreviewButtonClicked)
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Top)
						[
							SNew(STextBlock)
							.TextStyle( FEditorStyle::Get(), FEditorStyle::Join( "ViewportMenu", ".Label" ) )
							.Text(this, &SSCSUIEditorViewportToolBar::GetLocalizationPreviewLabel)
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Bottom)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.FillWidth(1.0f)

							+SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew( SBox )
								.WidthOverride( 4 )
								.HeightOverride( 4 )
								[
									SNew(SImage)
									.Image(FEditorStyle::GetBrush("ComboButton.Arrow"))
									.ColorAndOpacity(FLinearColor::Black)
								]
							]

							+SHorizontalBox::Slot()
								.FillWidth(1.0f)
						]
					]
				]*/
				+ SHorizontalBox::Slot()
				.Padding(2.0f, 2.0f)
				.AutoWidth()
				.VAlign(VAlign_Fill)
				[
					SNew(SCheckBox)
					.Cursor( EMouseCursor::Default )
					.Padding(FMargin( 4.0f ))
					.Style(FEditorStyle::Get(), EMultiBlockLocation::ToName(CheckboxStyle, EMultiBlockLocation::Type::None))
					.OnCheckStateChanged(this, &SSCSUIEditorViewportToolBar::HandleToggleShowStats)
					.ToolTipText(NSLOCTEXT("DesignerCommands", "Toggle Stats Preview", "Enables or disables the Stats preview for the current preview ."))
					.IsChecked(this, &SSCSUIEditorViewportToolBar::IsShowStats)
					.Content()
					[
						SNew( SBox )
						.WidthOverride( 40 )
						.HeightOverride( 16 )
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("UIViewportViewMode_Stats", "  Stats  "))
						]
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(2, 2.0f)
				.AutoWidth()
				.VAlign(VAlign_Fill)
				[
					SNew(SCheckBox)
					.Cursor(EMouseCursor::Default)
					.Padding(FMargin(4, 4))
					.Style(FEditorStyle::Get(), EMultiBlockLocation::ToName(CheckboxStyle, EMultiBlockLocation::Type::None))
					.OnCheckStateChanged(this, &SSCSUIEditorViewportToolBar::HandleToggleVisibility)
					.ToolTipText(LOCTEXT("VisibilityOptionIcon", "When toggled on, enable visibility for editor. When toggled off, disable visibility for editor"))
					.IsChecked(this, &SSCSUIEditorViewportToolBar::IsVisibilityChecked)
					.HAlign(HAlign_Center)
					.Content()
					[
						SNew(SBox)
						.WidthOverride(16)
						.HeightOverride(16)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.Padding(0)
							.VAlign(VAlign_Center)
							[
								SNew(SImage)
								.Image(GetVisibilityIcon())
							]
						]
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(0, 2.0f)
				.AutoWidth()
				.VAlign(VAlign_Fill)
				[
					SNew(SCheckBox)
					.Cursor(EMouseCursor::Default)
					.Padding(FMargin(4, 4))
					.Style(FEditorStyle::Get(), EMultiBlockLocation::ToName(CheckboxStyle, EMultiBlockLocation::Type::None))
					.OnCheckStateChanged(this, &SSCSUIEditorViewportToolBar::HandleToggleRespectLock)
					.ToolTipText(LOCTEXT("Respect Locks", "Enables or disables respecting locks placed on widgets.  Normally locked widgets prevent being selected in the designer."))
					.IsChecked(this, &SSCSUIEditorViewportToolBar::IsRespectLockChecked)
					.Content()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.Padding(0)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(GetRespectLockIcon())
						]
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(0, 2.0f)
				.AutoWidth()
				.VAlign(VAlign_Fill)
				[
					SNew(SCheckBox)
					.Cursor(EMouseCursor::Default)
					.Padding(FMargin(4, 4))
					.Style(FEditorStyle::Get(), EMultiBlockLocation::ToName(CheckboxStyle, EMultiBlockLocation::Type::None))
					.OnCheckStateChanged(this, &SSCSUIEditorViewportToolBar::HandleToggleShowRaycast)
					.ToolTipText(LOCTEXT("UIShowRaycastMode_ToolTip", "When toggled on, show the raycast regions. When toggled off, hide the raycast regions"))
					.IsChecked(this, &SSCSUIEditorViewportToolBar::IsShowRaycastChecked)
					.Content()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.Padding(0)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(GetRaycastIcon())
						]
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(0, 2.0f)
				.AutoWidth()
				.VAlign(VAlign_Fill)
				[
					SNew(SCheckBox)
					.Cursor(EMouseCursor::Default)
					.Padding(FMargin(4, 4))
					.Style(FEditorStyle::Get(), EMultiBlockLocation::ToName(CheckboxStyle, EMultiBlockLocation::Type::None))
					.OnCheckStateChanged(this, &SSCSUIEditorViewportToolBar::HandleShowToggleBackgroundImage)
					.ToolTipText(LOCTEXT("UIShowBackGroundImage_ToolTip", "When toggled on, show the preview background. When toggled off, hide the preview background"))
					.IsChecked(this, &SSCSUIEditorViewportToolBar::IsShowBackGroundImage)
					.Content()
					[
					    SNew(SHorizontalBox)
					    + SHorizontalBox::Slot()
					    .Padding(0)
					    .VAlign(VAlign_Center)
					    [
							SNew(SImage)
							.Image(GetPreviewBackgroundIcon())
					    ]
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(2.0f, 2.0f)
				.AutoWidth()
				.VAlign(VAlign_Fill)
				[
					SNew(SCheckBox)
					.Cursor(EMouseCursor::Default)
					.Padding(FMargin(4, 4))
					.Style(FEditorStyle::Get(), EMultiBlockLocation::ToName(CheckboxStyle, EMultiBlockLocation::Type::None))
					.OnCheckStateChanged(this, &SSCSUIEditorViewportToolBar::HandleToggleRawEditMode)
					.ToolTipText(LOCTEXT("UIRawEditMode_ToolTip", "Raw edit mode.When enabled,editing pivot and anchor values will not counter-adjust the position and size of the rectangle in order to make it stay in place."))
					.IsChecked(this, &SSCSUIEditorViewportToolBar::IsRawEditModeChecked)
					.Content()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.Padding(0)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(GetRawEditModeIcon())
						]
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(0, 2.0f)
				.AutoWidth()
				.VAlign(VAlign_Fill)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "ViewportMenu.Button")
					.ToolTipText(LOCTEXT("ZoomToFit_ToolTip", "Zoom To Fit"))
					.OnClicked(this, &SSCSUIEditorViewportToolBar::HandleResetZoom)
					.ContentPadding(FEditorStyle::Get().GetMargin("ViewportMenu.SToolBarButtonBlock.Button.Padding"))
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("UMGEditor.ZoomToFit"))
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(2.0f, 1.0f)
				.AutoWidth()
				[
					SNew(SViewportToolBarIconMenu)
					.Cursor(EMouseCursor::Default)
					.Style(FName("ViewportMenu"))
					.Label(this, &SSCSUIEditorViewportToolBar::GetCameraSpeedLabel)
					.OnGetMenuContent(this, &SSCSUIEditorViewportToolBar::FillCameraSpeedMenu)
					.ToolTipText(LOCTEXT("CameraSpeed_ToolTip", "Camera Speed"))
					.Icon(FSlateIcon(FEditorStyle::GetStyleSetName(), "EditorViewport.CamSpeedSetting"))
					.ParentToolBar(SharedThis(this))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("CameraSpeedButton")))
				]
			]
		];

		SViewportToolBar::Construct(SViewportToolBar::FArguments());
	}

	void HandleToggleShowStats(ECheckBoxState InState) const
	{
		if(EditorViewport.IsValid())
		{
			EditorViewport.Pin()->ToggleShowStats();
		}
	}

	ECheckBoxState  IsShowStats() const
	{
		return EditorViewport.IsValid() && EditorViewport.Pin()->GetShowStats() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	
	void HandleToggleLocalizationPreview(ECheckBoxState InState)
	{
		if (InState == ECheckBoxState::Checked)
		{
			FTextLocalizationManager::Get().EnableGameLocalizationPreview();
		}
		else
		{
			FTextLocalizationManager::Get().DisableGameLocalizationPreview();
		}
	}

	ECheckBoxState IsLocalizationPreviewChecked() const
	{
		return FTextLocalizationManager::Get().IsGameLocalizationPreviewEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	const FSlateBrush* GetLocalizationIcon()
	{
		return FEditorStyle::Get().GetBrush(TEXT("WidgetDesigner.ToggleLocalizationPreview"));
	}

	FText GetLocalizationPreviewLabel() const
	{
		const FString PreviewGameLanguage = FTextLocalizationManager::Get().GetConfiguredGameLocalizationPreviewLanguage();
		return PreviewGameLanguage.IsEmpty() ? LOCTEXT("LocalizationPreviewLanguage_None", "None") : FText::AsCultureInvariant(PreviewGameLanguage);
	}
	 
	const FSlateBrush* GetRawEditModeIcon()
	{
		const auto ImageBrush = FUIBlueprintEditorStyle::Get().GetBrush("RawEditBlack");
		return ImageBrush;
	}

	const FSlateBrush* GetPreviewBackgroundIcon()
	{
		const auto ImageBrush = FUIBlueprintEditorStyle::Get().GetBrush("PreviewBackground");
		return ImageBrush;
	}

	const FSlateBrush* GetRaycastIcon()
	{
		return FUIBlueprintEditorStyle::Get().GetBrush("RaycastRegion");
	}

	const FSlateBrush* GetRespectLockIcon()
	{
		return FEditorStyle::Get().GetBrush("WidgetDesigner.ToggleRespectLocks");
	}
	
	FText GetCameraSpeedLabel() const
	{
		const auto ViewportPin = EditorViewport.Pin();
		if (ViewportPin.IsValid() && ViewportPin->GetViewportClient().IsValid())
		{
			return FText::AsNumber(ViewportPin->GetViewportClient()->GetCameraSpeedSetting());
		}

		return FText();
	}

	/** Creates the preview menu */
	TSharedRef<SWidget> GeneratePreviewMenu() const
	{
		const TSharedPtr<const FUICommandList> CommandList = EditorViewport.IsValid()? EditorViewport.Pin()->GetCommandList(): NULL;

		constexpr bool bInShouldCloseWindowAfterMenuSelection = true;

		FMenuBuilder PreviewOptionsMenuBuilder(bInShouldCloseWindowAfterMenuSelection, CommandList);
		{
			PreviewOptionsMenuBuilder.BeginSection("BlueprintEditorPreviewOptions", NSLOCTEXT("BlueprintEditor", "PreviewOptionsMenuHeader", "Preview Viewport Options"));
			{
				PreviewOptionsMenuBuilder.AddMenuEntry(FUIBlueprintEditorCommands::Get().ResetCamera);
				PreviewOptionsMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().ToggleRealTime);
				PreviewOptionsMenuBuilder.AddMenuEntry(FUIBlueprintEditorCommands::Get().TrackSelectedComponent);
			}
			PreviewOptionsMenuBuilder.EndSection();
		}

		return PreviewOptionsMenuBuilder.MakeWidget();
	}
	
	FText GetViewMenuLabel() const
	{
		FText Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_Default", "View");

		if (EditorViewport.IsValid())
		{
			switch (EditorViewport.Pin()->GetViewportClient()->GetViewMode())
			{
			case VMI_Lit:
				Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_Lit", "Lit");
				break;

			case VMI_Unlit:
				Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_Unlit", "Unlit");
				break;

			case VMI_BrushWireframe:
				Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_Wireframe", "Wireframe");
				break;
			default: ;
			}
		}

		return Label;
	}

	const FSlateBrush* GetViewMenuLabelIcon() const
	{
		static FName LitModeIconName("EditorViewport.LitMode");
		static FName UnlitModeIconName("EditorViewport.UnlitMode");
		static FName WireframeModeIconName("EditorViewport.WireframeMode");

		FName Icon = NAME_None;

		if (EditorViewport.IsValid())
		{
			switch (EditorViewport.Pin()->GetViewportClient()->GetViewMode())
			{
			case VMI_Lit:
				Icon = LitModeIconName;
				break;

			case VMI_Unlit:
				Icon = UnlitModeIconName;
				break;

			case VMI_BrushWireframe:
				Icon = WireframeModeIconName;
				break;
			default: ;
			}
		}

		return FEditorStyle::GetBrush(Icon);
	}

	TSharedRef<SWidget> GenerateViewMenu() const
	{
		TSharedPtr<const FUICommandList> CommandList = EditorViewport.IsValid() ? EditorViewport.Pin()->GetCommandList() : nullptr;

		const bool bInShouldCloseWindowAfterMenuSelection = true;
		FMenuBuilder ViewMenuBuilder(bInShouldCloseWindowAfterMenuSelection, CommandList);

		ViewMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().LitMode, NAME_None, NSLOCTEXT("BlueprintEditor", "LitModeMenuOption", "Lit"));
		ViewMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().UnlitMode, NAME_None, NSLOCTEXT("BlueprintEditor", "UnlitModeMenuOption", "Unlit"));
		ViewMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().WireframeMode, NAME_None, NSLOCTEXT("BlueprintEditor", "WireframeModeMenuOption", "Wireframe"));

		return ViewMenuBuilder.MakeWidget();
	}

	ECheckBoxState Is2DViewportViewModeChecked() const
	{
		return EditorViewport.IsValid() && EditorViewport.Pin()->Is2DViewportType() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	void HandleToggleViewportViewMode(ECheckBoxState InState)
	{
		if (EditorViewport.IsValid())
		{
			EditorViewport.Pin()->Toggle2DViewMode(InState == ECheckBoxState::Checked);
		}
	}

	ECheckBoxState IsShowRaycastChecked() const
	{
		return EditorViewport.IsValid() && EditorViewport.Pin()->IsShowRaycast() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	ECheckBoxState IsRespectLockChecked() const
	{
		return GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRespectLock ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	
	FText GetViewportViewModeLabel() const
	{
		return LOCTEXT("UIViewportViewMode_2D", "    2D    ");
	}

	ECheckBoxState IsSimulateGameOverlayChecked() const
	{
		return EditorViewport.IsValid() && EditorViewport.Pin()->IsSimulateGameOverlay() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	FText GetRaycastLabel() const
	{
		return LOCTEXT("UIShowRaycastRegion", "  Raycast Regions  ");
	}

	FText GetShowBackgroundImageLabel() const
	{
		return LOCTEXT("UIShowBackgroundImage", "  Background Image  ");
	}

	ECheckBoxState IsVisibilityChecked() const
	{
		return GetMutableDefault<UUGUISettings>()->bVisibilityForEditor ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	
	void HandleToggleVisibility(ECheckBoxState InState) const
	{
		auto* Settings = GetMutableDefault<UUGUISettings>();

		bool bVisibilityForEditor = Settings->bVisibilityForEditor;

		bVisibilityForEditor = !bVisibilityForEditor;
		Settings->bVisibilityForEditor = bVisibilityForEditor;
		Settings->PostEditChange();

		if (EditorViewport.IsValid())
		{
			EditorViewport.Pin()->UpdatePreviewActorVisibility();
		}
	}

	const FSlateBrush* GetVisibilityIcon()
	{
		return FEditorStyle::Get().GetBrush("GenericCurveEditor.VisibilityOptions.Small");
	}
	
	void HandleToggleRespectLock(ECheckBoxState InState) const
	{
		auto* Settings = GetMutableDefault<UUIEditorPerProjectUserSettings>();

		bool bRespectLock = Settings->bRespectLock;

		bRespectLock = !bRespectLock;
		Settings->bRespectLock = bRespectLock;
		Settings->PostEditChange();
	}

	void HandleToggleShowRaycast(ECheckBoxState InState) const
	{
		if (EditorViewport.IsValid())
		{
			EditorViewport.Pin()->ToggleShowRaycast();
		}
	}

	void HandleToggleRawEditMode(ECheckBoxState InState) const 
	{
	    if(EditorViewport.IsValid())
	    {
			EditorViewport.Pin()->ToggleRawEditMode();
	    }
	}

	void HandleShowToggleBackgroundImage(ECheckBoxState InState)const
	{
	    if(EditorViewport.IsValid())
	    {
			EditorViewport.Pin()->ToggleShowBackgroundImage();
	    }
	}

	FReply HandleResetZoom() const
	{
		if (EditorViewport.IsValid())
		{
			EditorViewport.Pin()->ResetZoom();
			return FReply::Handled();
		}

	    return FReply::Unhandled();
	}

	ECheckBoxState IsShowBackGroundImage()const
	{
		return EditorViewport.IsValid() && EditorViewport.Pin()->GetShowBackgroundImage() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	ECheckBoxState IsRawEditModeChecked() const
	{
		return EditorViewport.IsValid() && EditorViewport.Pin()->GetRawEditMode() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	void HandleToggleSimulateGameOverlay(ECheckBoxState InState) const
	{
		if (EditorViewport.IsValid())
		{
			EditorViewport.Pin()->ToggleOverlay(InState == ECheckBoxState::Checked);
		}
	}

	FText GetIsSimulateGameOverlayLabel() const
	{
		return LOCTEXT("UIViewportSimulateGameOverlay", "  Overlay  ");
	}

	float GetCamSpeedSliderPosition() const
	{
		float SliderPos = 0.f;

		auto ViewportPin = EditorViewport.Pin();
		if (ViewportPin.IsValid() && ViewportPin->GetViewportClient().IsValid())
		{
			SliderPos = (ViewportPin->GetViewportClient()->GetCameraSpeedSetting() - 1) / ((float)FEditorViewportClient::MaxCameraSpeeds - 1);
		}

		return SliderPos;
	}

	void OnSetCamSpeed(float NewValue)
	{
		const auto ViewportPin = EditorViewport.Pin();
		if (ViewportPin.IsValid() && ViewportPin->GetViewportClient().IsValid())
		{
			const int32 OldSpeedSetting = ViewportPin->GetViewportClient()->GetCameraSpeedSetting();
			const int32 NewSpeedSetting = NewValue * ((float)FEditorViewportClient::MaxCameraSpeeds - 1) + 1;

			if (OldSpeedSetting != NewSpeedSetting)
			{
				ViewportPin->GetViewportClient()->SetCameraSpeedSetting(NewSpeedSetting);
				OnCamSpeedChanged.ExecuteIfBound(NewSpeedSetting);
			}
		}
	}

	TSharedRef<SWidget> FillCameraSpeedMenu()
	{
		TSharedRef<SWidget> ReturnWidget = SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush(TEXT("Menu.Background")))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
			    .AutoHeight()
			    .Padding(FMargin(8.0f, 2.0f, 60.0f, 2.0f))
			    .HAlign(HAlign_Left)
			    [
				    SNew(STextBlock)
				    .Text(LOCTEXT("MouseSettingsCamSpeed", "Camera Speed"))
			        .Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
			    ]
		    + SVerticalBox::Slot()
			    .AutoHeight()
			    .Padding(FMargin(8.0f, 4.0f))
			    [
				    SNew(SHorizontalBox)
				    + SHorizontalBox::Slot()
			    .FillWidth(1)
			    .Padding(FMargin(0.0f, 2.0f))
			    [
					SAssignNew(CamSpeedSlider, SSlider)
				    .Value(this, &SSCSUIEditorViewportToolBar::GetCamSpeedSliderPosition)
			        .OnValueChanged(this, &SSCSUIEditorViewportToolBar::OnSetCamSpeed)
			    ]
		    + SHorizontalBox::Slot()
			    .AutoWidth()
			    .Padding(8.0f, 2.0f, 0.0f, 2.0f)
			    [
				    SNew(STextBlock)
				    .Text(this, &SSCSUIEditorViewportToolBar::GetCameraSpeedLabel)
			        .Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
			    ]
			] // Camera Speed Scalar
		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(8.0f, 2.0f, 60.0f, 2.0f))
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MouseSettingsCamSpeedScalar", "Camera Speed Scalar"))
			    .Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
			]
		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(8.0f, 4.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
			    .FillWidth(1)
			    .Padding(FMargin(0.0f, 2.0f))
			    [
				    SAssignNew(CamSpeedScalarBox, SSpinBox<float>)
				    .MinValue(1)
			        .MaxValue(TNumericLimits<int32>::Max())
			        .MinSliderValue(1)
			        .MaxSliderValue(128)
			        .Value(this, &SSCSUIEditorViewportToolBar::GetCamSpeedScalarBoxValue)
			        .OnValueChanged(this, &SSCSUIEditorViewportToolBar::OnSetCamSpeedScalarBoxValue)
			        .ToolTipText(LOCTEXT("CameraSpeedScalar_ToolTip", "Scalar to increase camera movement range"))
			    ]
			]
			];

		return ReturnWidget;
	}

	float GetCamSpeedScalarBoxValue() const
	{
		float CamSpeedScalar = 1.f;

		auto ViewportPin = EditorViewport.Pin();
		if (ViewportPin.IsValid() && ViewportPin->GetViewportClient().IsValid())
		{
			CamSpeedScalar = (ViewportPin->GetViewportClient()->GetCameraSpeedScalar());
		}

		return CamSpeedScalar;
	}

	void OnSetCamSpeedScalarBoxValue(float NewValue)
	{
		auto ViewportPin = EditorViewport.Pin();
		if (ViewportPin.IsValid() && ViewportPin->GetViewportClient().IsValid())
		{
			ViewportPin->GetViewportClient()->SetCameraSpeedScalar(NewValue);
			OnCamSpeedScalarChanged.ExecuteIfBound(NewValue);
		}
	}

private:
	/** Reference to the parent viewport */
	TWeakPtr<SSCSUIEditorViewport> EditorViewport;

	FOnCamSpeedChanged OnCamSpeedChanged;
	FOnCamSpeedScalarChanged OnCamSpeedScalarChanged;

	/** Reference to the camera slider used to display current camera speed */
	mutable TSharedPtr< SSlider > CamSpeedSlider;

	/** Reference to the camera spinbox used to display current camera speed scalar */
	mutable TSharedPtr< SSpinBox<float> > CamSpeedScalarBox;
};


/*-----------------------------------------------------------------------------
   SSCSUIEditorViewport
-----------------------------------------------------------------------------*/

void SSCSUIEditorViewport::Construct(const FArguments& InArgs)
{
	bIsActiveTimerRegistered = false;
	bSimulateGameOverlay = false;
	LastCanvasRenderMode = ECanvasRenderMode::CanvasRenderMode_ScreenSpaceFree;
	
	// Save off the Blueprint editor reference, we'll need this later
	BlueprintEditorPtr = InArgs._BlueprintEditor;

	SEditorViewport::Construct( SEditorViewport::FArguments() );

	// Restore last used feature level
	if (ViewportClient.IsValid())
	{
		UWorld* World = ViewportClient->GetPreviewScene()->GetWorld();
		if (World != nullptr)
		{
			World->ChangeFeatureLevel(GWorld->FeatureLevel);
		}
	}

	// Use a delegate to inform the attached world of feature level changes.
	UEditorEngine* Editor = (UEditorEngine*)GEngine;
	PreviewFeatureLevelChangedHandle = Editor->OnPreviewFeatureLevelChanged().AddLambda([this](ERHIFeatureLevel::Type NewFeatureLevel)
		{
			if(ViewportClient.IsValid())
			{
				UWorld* World = ViewportClient->GetPreviewScene()->GetWorld();
				if (World != nullptr)
				{
					World->ChangeFeatureLevel(NewFeatureLevel);

					// Refresh the preview scene. Don't change the camera.
					RequestRefresh(false);
				}
			}
		});

	// Refresh the preview scene
	RequestRefresh(true);
}

SSCSUIEditorViewport::~SSCSUIEditorViewport()
{
	UEditorEngine* Editor = (UEditorEngine*)GEngine;
	Editor->OnPreviewFeatureLevelChanged().Remove(PreviewFeatureLevelChangedHandle);

	if(ViewportClient.IsValid())
	{
		// Reset this to ensure it's no longer in use after destruction
		ViewportClient->Viewport = NULL;
	}
}

bool SSCSUIEditorViewport::IsVisible() const
{
	// We consider the viewport to be visible if the reference is valid
	return ViewportWidget.IsValid() && SEditorViewport::IsVisible();
}

TSharedRef<FEditorViewportClient> SSCSUIEditorViewport::MakeEditorViewportClient()
{
	FPreviewScene* PreviewScene = BlueprintEditorPtr.Pin()->GetPreviewScene();

	// Construct a new viewport client instance.
	ViewportClient = MakeShareable(new FSCSUIEditorViewportClient(BlueprintEditorPtr, PreviewScene, SharedThis(this)));
	ViewportClient->SetRealtime(true);
	ViewportClient->bSetListenerPosition = false;
	ViewportClient->VisibilityDelegate.BindSP(this, &SSCSUIEditorViewport::IsVisible);
	UpdateCanvasRenderMode();

	if (PreviewScene && PreviewScene->GetWorld())
	{
		for (int32 Index = 0, Count = PreviewScene->GetWorld()->ExtraReferencedObjects.Num(); Index < Count; ++Index)
		{
			const auto& EventViewportClient = Cast<UDesignerEditorEventViewportClient>(PreviewScene->GetWorld()->ExtraReferencedObjects[Index]);
			if (EventViewportClient)
			{
				EventViewportClient->ViewportClient = ViewportClient;
				ViewportClient->EventViewportClient = EventViewportClient;
				break;
			}
		}
	}
	
	return ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SSCSUIEditorViewport::MakeViewportToolbar()
{
	return SEditorViewport::MakeViewportToolbar();
}

void SSCSUIEditorViewport::PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay)
{
	Overlay->AddSlot()
		.VAlign(VAlign_Top)
		[
			SNew(SSCSUIEditorViewportToolBar)
			.EditorViewport(SharedThis(this))
			.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
		];

	SEditorViewport::PopulateViewportOverlays(Overlay);
	
	Overlay->AddSlot()
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Left)
	.Padding(FMargin(10.0f, 40.0f, 10.0f, 10.0f))
	[
		SAssignNew(OverlayTextVerticalBox, SVerticalBox)
	];

	// add the feature level display widget
	Overlay->AddSlot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Right)
		.Padding(5.0f)
		[
			BuildFeatureLevelWidget()
		];
}

void SSCSUIEditorViewport::BindCommands()
{
	FSCSUIEditorViewportCommands::Register(); // make sure the viewport specific commands have been registered

	TSharedPtr<FUIBlueprintEditor> BlueprintEditor = BlueprintEditorPtr.Pin();
	TSharedPtr<SSCSComponentEditor> SCSEditorWidgetPtr = BlueprintEditor->GetSCSComponentEditor();
	SSCSComponentEditor* SCSEditorWidget = SCSEditorWidgetPtr.Get();
	// for mac, we have to bind a command that would override the BP-Editor's 
	// "NavigateToParentBackspace" command, because the delete key is the 
	// backspace key for that platform (and "NavigateToParentBackspace" does not 
	// make sense in the viewport window... it blocks the generic delete command)
	// 
	// NOTE: this needs to come before we map any other actions (so it is 
	// prioritized first)
	CommandList->MapAction(
		FSCSUIEditorViewportCommands::Get().DeleteComponent,
		FExecuteAction::CreateSP(SCSEditorWidget, &SSCSComponentEditor::OnDeleteNodes),
		FCanExecuteAction::CreateSP(SCSEditorWidget, &SSCSComponentEditor::CanDeleteNodes)
	);

	CommandList->Append(BlueprintEditor->GetSCSComponentEditor()->CommandList.ToSharedRef());
	CommandList->Append(BlueprintEditor->GetToolkitCommands());
	SEditorViewport::BindCommands();

	const FUIBlueprintEditorCommands& Commands = FUIBlueprintEditorCommands::Get();

	BlueprintEditorPtr.Pin()->GetToolkitCommands()->MapAction(
		Commands.EnableSimulation,
		FExecuteAction::CreateSP(this, &SSCSUIEditorViewport::ToggleIsSimulateEnabled),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(ViewportClient.Get(), &FSCSUIEditorViewportClient::GetIsSimulateEnabled),
		FIsActionButtonVisible::CreateSP(this, &SSCSUIEditorViewport::ShouldShowViewportCommands));

	// Toggle camera lock on/off
	CommandList->MapAction(
		Commands.ResetCamera,
		FExecuteAction::CreateSP(ViewportClient.Get(), &FSCSUIEditorViewportClient::ResetCamera) );

	CommandList->MapAction(
		Commands.TrackSelectedComponent,
		FExecuteAction::CreateSP(ViewportClient.Get(), &FSCSUIEditorViewportClient::ToggleTrackSelectedComponent),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(ViewportClient.Get(),&FSCSUIEditorViewportClient::GetTrackSelectedComponent)
	);
}

void SSCSUIEditorViewport::Invalidate()
{
	ViewportClient->Invalidate();
}

void SSCSUIEditorViewport::ToggleIsSimulateEnabled()
{
	// Make the viewport visible if the simulation is starting.
	if ( !ViewportClient->GetIsSimulateEnabled() )
	{
		if ( GetDefault<UBlueprintEditorSettings>()->bShowViewportOnSimulate )
		{
			BlueprintEditorPtr.Pin()->GetTabManager()->TryInvokeTab(FBlueprintEditorTabs::SCSViewportID);
		}
	}

	ViewportClient->ToggleIsSimulateEnabled();
}

void SSCSUIEditorViewport::UpdateCanvasRenderMode()
{
	ECanvasRenderMode NewRenderMode = ECanvasRenderMode::CanvasRenderMode_ScreenSpaceFree;

	if (IsSimulateGameOverlay())
	{
		NewRenderMode = ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay;
	}
	else if (Is2DViewportType())
	{
		NewRenderMode = ECanvasRenderMode::CanvasRenderMode_ScreenSpaceFree;
	}

	bool bNeedRefresh = false;
	if (LastCanvasRenderMode != NewRenderMode)
	{
		bNeedRefresh = true;
		LastCanvasRenderMode = NewRenderMode;
	}
	
#if WITH_EDITOR
	if (ViewportClient.IsValid() && ViewportClient->GetPreviewScene())
	{
		FWorldViewportInfo* WorldViewportInfo = UUGUISubsystem::GetWorldViewportInfo(ViewportClient->GetPreviewScene()->GetWorld(), true);
		if (WorldViewportInfo)
		{
			WorldViewportInfo->SetRenderMode(ViewportClient->GetPreviewScene()->GetWorld(), NewRenderMode);
		}
	}
#endif

	if (bNeedRefresh)
	{
		RequestRefresh(false, true);
	}
}

void SSCSUIEditorViewport::EnablePreview(bool bEnable)
{
	const FText SystemDisplayName = NSLOCTEXT("BlueprintEditor", "RealtimeOverrideMessage_Blueprints", "the active blueprint mode");
	if(bEnable)
	{
		// Restore the previously-saved realtime setting
		ViewportClient->RemoveRealtimeOverride(SystemDisplayName);
	}
	else
	{
		// Disable and store the current realtime setting. This will bypass real-time rendering in the preview viewport (see UEditorEngine::UpdateSingleViewportClient).
		const bool bShouldBeRealtime = false;
		ViewportClient->AddRealtimeOverride(bShouldBeRealtime, SystemDisplayName);
	}
}

void SSCSUIEditorViewport::RequestRefresh(bool bResetCamera, bool bRefreshNow)
{
	if(bRefreshNow)
	{
		if(ViewportClient.IsValid())
		{
			ViewportClient->InvalidatePreview(bResetCamera);
		}
	}
	else
	{
		// Defer the update until the next tick. This way we don't accidentally spawn the preview actor in the middle of a transaction, for example.
		if (!bIsActiveTimerRegistered)
		{
			bIsActiveTimerRegistered = true;
			RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SSCSUIEditorViewport::DeferredUpdatePreview, bResetCamera));
		}
	}
}

void SSCSUIEditorViewport::OnComponentSelectionChanged()
{
	// When the component selection changes, make sure to invalidate hit proxies to sync with the current selection
	SceneViewport->Invalidate();
}

void SSCSUIEditorViewport::OnFocusViewportToSelection()
{
	ViewportClient->FocusViewportToSelection();
}

bool SSCSUIEditorViewport::ShouldShowViewportCommands() const
{
	// Hide if actively debugging
	return !GIntraFrameDebuggingGameThread;
}

bool SSCSUIEditorViewport::GetIsSimulateEnabled()
{
	return ViewportClient->GetIsSimulateEnabled();
}

void SSCSUIEditorViewport::ToggleShowRaycast()
{
	ViewportClient->ToggleShowRaycastRegion();
}

void SSCSUIEditorViewport::ToggleRawEditMode()
{
	ViewportClient->ToggleRawEditMode();
}

bool SSCSUIEditorViewport::GetRawEditMode()
{
	return ViewportClient->GetRawEditMode();
}

void SSCSUIEditorViewport::ToggleShowBackgroundImage()
{
	ViewportClient->ToggleShowBackgroundImage();
}

bool SSCSUIEditorViewport::GetShowBackgroundImage()
{
	return ViewportClient->GetShowBackgroundImage();
}

void SSCSUIEditorViewport::ResetZoom()
{
	ViewportClient->ResetZoom();
}

void SSCSUIEditorViewport::ToggleShowStats()
{
	ViewportClient->ToggleShowStats();
}

bool SSCSUIEditorViewport::GetShowStats()
{
	return ViewportClient->GetShowStats();
}

void SSCSUIEditorViewport::PopulateOverlayText(const TArray<FOverlayTextItem>& TextItems)
{
	OverlayTextVerticalBox->ClearChildren();

	for (const auto& TextItem : TextItems)
	{
		OverlayTextVerticalBox->AddSlot()
		.Padding(0, 0, 0, 4)
		[
			SNew(STextBlock)
			.Text(TextItem.Text)
			.TextStyle(FEditorStyle::Get(), TextItem.Style)
		];
	}
}

void SSCSUIEditorViewport::SetOwnerTab(TSharedRef<SDockTab> Tab)
{
	OwnerTab = Tab;
}

TSharedPtr<SDockTab> SSCSUIEditorViewport::GetOwnerTab() const
{
	return OwnerTab.Pin();
}

void SSCSUIEditorViewport::Toggle2DViewMode(bool b2DViewMode)
{
	if (ViewportClient.IsValid())
	{
		if (b2DViewMode)
		{
			ViewportClient->ViewportType = ELevelViewportType::LVT_OrthoXZ;
		}
		else
		{
			ViewportClient->ViewportType = ELevelViewportType::LVT_Perspective;
		}
	}
	
	UpdateCanvasRenderMode();
}

void SSCSUIEditorViewport::ToggleOverlay(bool bOverlay)
{
	bSimulateGameOverlay = bOverlay;
	UpdateCanvasRenderMode();
}

void SSCSUIEditorViewport::UpdatePreviewActorVisibility()
{
	if (BlueprintEditorPtr.IsValid())
	{
		BlueprintEditorPtr.Pin()->UpdatePreviewWidgetActor(BlueprintEditorPtr.Pin()->GetBlueprintObj(), true);
	}
}

FReply SSCSUIEditorViewport::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	const TSharedPtr<SSCSComponentEditor> SCSComponentEditor = BlueprintEditorPtr.Pin()->GetSCSComponentEditor();
	return SCSComponentEditor->TryHandleAssetDragDropOperation(DragDropEvent);
}

EActiveTimerReturnType SSCSUIEditorViewport::DeferredUpdatePreview(double InCurrentTime, float InDeltaTime, bool bResetCamera)
{
	if (ViewportClient.IsValid())
	{
		ViewportClient->InvalidatePreview(bResetCamera);
	}

	bIsActiveTimerRegistered = false;
	return EActiveTimerReturnType::Stop;
}

#undef LOCTEXT_NAMESPACE
