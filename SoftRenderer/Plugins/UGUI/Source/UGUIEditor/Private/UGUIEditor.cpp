#include "UGUIEditor.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "ISequencerModule.h"
#include "ISettingsModule.h"
#include "UGUIEditorStyle.h"
#include "UGUISettings.h"
#include "Core/Layout/AspectRatioFitterSubComponent.h"
#include "Core/Layout/CanvasScalerSubComponent.h"
#include "Core/Layout/ContentSizeFitterSubComponent.h"
#include "Core/Layout/GridLayoutGroupComponent.h"
#include "Core/Layout/HorizontalLayoutGroupComponent.h"
#include "Core/Layout/LayoutElementSubComponent.h"
#include "Core/Layout/SafeZoneComponent.h"
#include "Core/Layout/VerticalLayoutGroupComponent.h"
#include "Core/VertexModifiers/OutlineSubComponent.h"
#include "Core/VertexModifiers/PositionAsUV1SubComponent.h"
#include "Core/VertexModifiers/ShadowSubComponent.h"
#include "Core/Widgets/BackgroundBlurComponent.h"
#include "Core/Widgets/BackgroundBlurSubComponent.h"
#include "Core/Widgets/BackgroundGlitchComponent.h"
#include "Core/Widgets/BackgroundGlitchSubComponent.h"
#include "Core/Widgets/ImageComponent.h"
#include "Core/Widgets/ButtonComponent.h"
#include "Core/Widgets/ImageSubComponent.h"
#include "Core/Widgets/RawImageComponent.h"
#include "Core/Widgets/RawImageSubComponent.h"
#include "Core/Widgets/RectMask2DSubComponent.h"
#include "Core/Widgets/Text/TextComponent.h"
#include "Core/Widgets/Text/TextSubComponent.h"
#include "Core/Widgets/FocusComponent.h"
#include "Core/Widgets/InputFieldComponent.h"
#include "Core/Widgets/SliderComponent.h"
#include "Core/Widgets/ToggleComponent.h"
#include "Core/Widgets/ToggleGroupComponent.h"
#include "Core/Widgets/ScrollRectComponent.h"
#include "Core/Widgets/ScrollbarComponent.h"
#include "Animation/UIAnimationAttachComponent.h"
#include "Animation/UIAnimationSlotComponent.h"
#include "EventSystem/EventSystemComponent.h"
#include "EventSystem/EventTriggerComponent.h"
#include "DetailCustomizations/BackgroundBlurComponentDetails.h"
#include "DetailCustomizations/BackgroundGlitchComponentDetails.h"
#include "DetailCustomizations/BehaviourComponentDetails.h"
#include "DetailCustomizations/GridLayoutGroupComponentDetails.h"
#include "DetailCustomizations/HorizontalLayoutGroupComponentDetails.h"
#include "DetailCustomizations/ImageComponentDetails.h"
#include "DetailCustomizations/RawImageComponentDetails.h"
#include "DetailCustomizations/SafeZoneComponentDetails.h"
#include "DetailCustomizations/VerticalLayoutGroupComponentDetails.h"
#include "DetailCustomizations/SubComponents/AspectRatioFitterSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/BackgroundBlurSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/BackgroundGlitchSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/CanvasRendererSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/CanvasScalerSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/CanvasSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/ContentSizeFitterSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/ImageSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/LayoutElementSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/OutlineSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/PositionAsUV1SubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/RawImageSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/RectMask2DSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/ShadowSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/TextSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/StandaloneInputModuleSubComponentCustomization.h"
#include "EventSystem/InputModules/StandaloneInputModuleSubComponent.h"
#include "SupportedRangeTypes.h"	// StructsSupportingRangeVisibility
#include "AssetTypeAction/AssetTypeAction_UIBlueprint.h"
#include "AssetTypeAction/AssetTypeAction_TextEmojiSheet.h"
#include "Core/Layout/ChildWidgetActorComponent.h"
#include "Core/Layout/GridLayoutGroupSubComponent.h"
#include "Core/Layout/HorizontalLayoutGroupSubComponent.h"
#include "Core/Layout/VerticalLayoutGroupSubComponent.h"
#include "Core/Widgets/FocusSubComponent.h"
#include "Core/Widgets/LineComponent.h"
#include "Core/Widgets/LineSubComponent.h"
#include "Core/Widgets/Cascade/UICascadeComponent.h"
#include "Core/Widgets/Cascade/UICascadeSubComponent.h"
#include "Core/Widgets/Mesh/UISimpleStaticMeshComponent.h"
#include "Core/Widgets/Mesh/UISimpleStaticMeshSubComponent.h"
#include "Core/Widgets/Mesh/UIStaticMeshComponent.h"
#include "Core/Widgets/Mesh/UIStaticMeshSubComponent.h"
#include "Core/Widgets/Niagara/UINiagaraComponent.h"
#include "Core/Widgets/Niagara/UINiagaraSubComponent.h"
#include "Core/Widgets/Text/TextHypertextClickSubComponent.h"
#include "Core/Widgets/CurveComponent.h"
#include "Customizations/MathStructCustomizations.h"
#include "DetailCustomizations/ChildWidgetActorComponentDetails.h"
#include "DetailCustomizations/LineComponentDetails.h"
#include "DetailCustomizations/RectTransformComponentDetails.h"
#include "DetailCustomizations/UICascadeComponentDetails.h"
#include "DetailCustomizations/UINiagaraComponentDetails.h"
#include "DetailCustomizations/UISimpleStaticMeshComponentDetails.h"
#include "DetailCustomizations/UIStaticMeshComponentDetails.h"
#include "DetailCustomizations/SubComponents/LineSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/UICascadeSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/UINiagaraSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/UISimpleStaticMeshSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/UIStaticMeshSubComponentCustomization.h"
#include "Sequence/TrackEditors/UIMaterialTrackEditor.h"
#include "DetailCustomizations/UIEnumModifierCustomization.h"
#include "DetailCustomizations/TextComponentDetails.h"
#include "DetailCustomizations/SubComponents/FocusSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/GridLayoutGroupSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/HorizontalLayoutGroupSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/TextHypertextClickSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/VerticalLayoutGroupSubComponentCustomization.h"
#include "Sequence/TrackEditors/PropertyTrackEditors/RotatorPropertyTrackEditor.h"
#include "DetailCustomizations/ButtonComponentDetails.h"
#include "DetailCustomizations/UIAnimationAttachComponentDetails.h"
#include "DetailCustomizations/UIAnimationSlotComponentDetails.h"
#include "DetailCustomizations/EventSystemComponentDetails.h"
#include "DetailCustomizations/EventTriggerComponentDetails.h"
#include "DetailCustomizations/CurveComponentDetails.h"
#include "DetailCustomizations/FocusComponentDetails.h"
#include "DetailCustomizations/InputFieldComponentDetails.h"
#include "DetailCustomizations/SliderComponentDetails.h"
#include "DetailCustomizations/ToggleComponentDetails.h"
#include "DetailCustomizations/ToggleGroupComponentDetails.h"
#include "DetailCustomizations/ScrollRectComponentDetails.h"
#include "DetailCustomizations/ScrollbarComponentDetails.h"
#include "DetailCustomizations/WidgetActorDetails.h"
#include "Animation/TweenAlphaSubComponent.h"
#include "Animation/TweenColorSubComponent.h"
#include "Animation/TweenPathSubComponent.h"
#include "Animation/TweenPositionSubComponent.h"
#include "Animation/TweenRotationSubComponent.h"
#include "Animation/TweenScaleSubComponent.h"
#include "Animation/TweenTransformSubComponent.h"
#include "Animation/TweenPlayerSubComponent.h"
#include "Core/Widgets/DropdownComponent.h"
#include "Core/Widgets/DropdownItemSubComponent.h"
#include "DetailCustomizations/DropdownComponentDetails.h"
#include "DetailCustomizations/SubComponents/DropdownItemSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/TweenBaseSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/TweenPathSubComponentCustomization.h"
#include "DetailCustomizations/SubComponents/TweenPlayerSubComponentCustomization.h"

#define LOCTEXT_NAMESPACE "FUGUIEditorModule"

void FUGUIEditorModule::StartupModule()
{
	FUGUIEditorStyle::Get();
	
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FUGUIEditorModule::OnPostEngineInit);

	IAssetTools& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	const auto AssetCategoryBit = AssetToolsModule.RegisterAdvancedAssetCategory(FName(TEXT("UGUI")),
		LOCTEXT("UGUICategory", "UGUI"));
	const TSharedPtr<FAssetTypeAction_UIBlueprint> UIBlueprintAssetTypeAction = MakeShareable(new FAssetTypeAction_UIBlueprint(AssetCategoryBit));
	ItemDataAssetTypeActions.Add(UIBlueprintAssetTypeAction);
	AssetToolsModule.RegisterAssetTypeActions(UIBlueprintAssetTypeAction.ToSharedRef());

	const TSharedPtr<FAssetTypeAction_TextEmojiSheet> EmojiActionType = MakeShareable(new FAssetTypeAction_TextEmojiSheet(AssetCategoryBit));
	ItemDataAssetTypeActions.Add(EmojiActionType);
	AssetToolsModule.RegisterAssetTypeActions(EmojiActionType.ToSharedRef());
	
	//register custom editor
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.RegisterCustomPropertyTypeLayout("UIEnumModifier", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FUIEnumModifierCustomization::MakeInstance));

		RangeVisibilityUtils::StructsSupportingRangeVisibility.Add("UIMargin");
		PropertyModule.RegisterCustomPropertyTypeLayout("UIMargin", 
			FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FMathStructCustomization::MakeInstance));

		RangeVisibilityUtils::StructsSupportingRangeVisibility.Add("UVRect");
		PropertyModule.RegisterCustomPropertyTypeLayout("UVRect",
			FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FMathStructCustomization::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(AWidgetActor::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FWidgetActorDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UBehaviourComponent::StaticClass()->GetFName(), 
			FOnGetDetailCustomizationInstance::CreateStatic(&FBehaviourComponentDetails::MakeInstance));
		
		PropertyModule.RegisterCustomClassLayout(UImageComponent::StaticClass()->GetFName(), 
			FOnGetDetailCustomizationInstance::CreateStatic(&FImageComponentDetails::MakeInstance));
		
		PropertyModule.RegisterCustomClassLayout(URawImageComponent::StaticClass()->GetFName(), 
			FOnGetDetailCustomizationInstance::CreateStatic(&FRawImageComponentDetails::MakeInstance));
		
		PropertyModule.RegisterCustomClassLayout(UGridLayoutGroupComponent::StaticClass()->GetFName(), 
			FOnGetDetailCustomizationInstance::CreateStatic(&FGridLayoutGroupComponentDetails::MakeInstance));
		
		PropertyModule.RegisterCustomClassLayout(UVerticalLayoutGroupComponent::StaticClass()->GetFName(), 
			FOnGetDetailCustomizationInstance::CreateStatic(&FVerticalLayoutGroupComponentDetails::MakeInstance));
		
		PropertyModule.RegisterCustomClassLayout(UHorizontalLayoutGroupComponent::StaticClass()->GetFName(), 
			FOnGetDetailCustomizationInstance::CreateStatic(&FHorizontalLayoutGroupComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(USafeZoneComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FSafeZoneComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UBackgroundBlurComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FBackgroundBlurComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UBackgroundGlitchComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FBackgroundGlitchComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUIStaticMeshComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FUIStaticMeshComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUISimpleStaticMeshComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FUISimpleStaticMeshComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUINiagaraComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FUINiagaraComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUICascadeComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FUICascadeComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(ULineComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FLineComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UTextComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FTextComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UChildWidgetActorComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FChildWidgetActorComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(URectTransformComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FRectTransformComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UButtonComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FButtonComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUIAnimationAttachComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FUIAnimationAttachComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UUIAnimationSlotComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FUIAnimationSlotComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UEventSystemComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FEventSystemComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UEventTriggerComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FEventTriggerComponentDetails::MakeInstance));
		
		PropertyModule.RegisterCustomClassLayout(UCurveComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FCurveComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UFocusComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FFocusComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UInputFieldComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FInputFieldComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(USliderComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FSliderComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UToggleComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FToggleComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UToggleGroupComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FToggleGroupComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UScrollRectComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FScrollRectComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UScrollbarComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FScrollbarComponentDetails::MakeInstance));

		PropertyModule.RegisterCustomClassLayout(UDropdownComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FDropdownComponentDetails::MakeInstance));
		
		PropertyModule.NotifyCustomizationModuleChanged();
	}

	//register sub component custom editor
	{
		RegisterSubComponentClassLayout(UCanvasScalerSubComponent::StaticClass()->GetFName(), 
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FCanvasScalerSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UAspectRatioFitterSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FAspectRatioFitterSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UContentSizeFitterSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FContentSizeFitterSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UOutlineSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FOutlineSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UPositionAsUV1SubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FPositionAsUV1SubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UShadowSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FShadowSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UStandaloneInputModuleSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FStandaloneInputModuleSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(URectMask2DSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FRectMask2DSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(ULayoutElementSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FLayoutElementSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UCanvasSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FCanvasSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UCanvasRendererSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FCanvasRendererSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(URawImageSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FRawImageSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UImageSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FImageSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UBackgroundBlurSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FBackgroundBlurSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UBackgroundGlitchSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FBackgroundGlitchSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UUIStaticMeshSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FUIStaticMeshSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UUISimpleStaticMeshSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FUISimpleStaticMeshSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UUINiagaraSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FUINiagaraSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UUICascadeSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FUICascadeSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(ULineSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FLineSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UTextSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FTextSubComponentCustomization::MakeInstance));
		
		RegisterSubComponentClassLayout(UHorizontalLayoutGroupSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FHorizontalLayoutGroupSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UVerticalLayoutGroupSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FVerticalLayoutGroupSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UGridLayoutGroupSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FGridLayoutGroupSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UTextHypertextClickSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FTextHypertextClickSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UFocusSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FFocusSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UTweenAlphaSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FTweenBaseSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UTweenColorSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FTweenBaseSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UTweenPathSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FTweenPathSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UTweenPositionSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FTweenBaseSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UTweenRotationSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FTweenBaseSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UTweenScaleSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FTweenBaseSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UTweenTransformSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FTweenBaseSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UTweenPlayerSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FTweenPlayerSubComponentCustomization::MakeInstance));

		RegisterSubComponentClassLayout(UDropdownItemSubComponent::StaticClass()->GetFName(),
			FOnGetSubComponentDetailCustomizationInstance::CreateStatic(&FDropdownItemSubComponentCustomization::MakeInstance));
	}

	if (GIsEditor)
	{
		ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");

		// register specialty track editors
		ComponentMaterialTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FUIComponentMaterialTrackEditor::CreateTrackEditor));
		RotatorPropertyTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FRotatorPropertyTrackEditor::CreateTrackEditor));
	}
}

void FUGUIEditorModule::ShutdownModule()
{
	FUGUIEditorStyle::Destroy();

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto& AssetTypeAction : ItemDataAssetTypeActions)
		{
			if (AssetTypeAction.IsValid())
			{
				AssetToolsModule.UnregisterAssetTypeActions(AssetTypeAction.ToSharedRef());
			}
		}
	}
	ItemDataAssetTypeActions.Empty();

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout("UIEnumModifier");
	PropertyModule.UnregisterCustomClassLayout(AWidgetActor::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UBehaviourComponent::StaticClass()->GetFName());	
	PropertyModule.UnregisterCustomClassLayout(UImageComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(URawImageComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UGridLayoutGroupComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UVerticalLayoutGroupComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UHorizontalLayoutGroupComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(USafeZoneComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UBackgroundBlurComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUIStaticMeshComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUISimpleStaticMeshComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUINiagaraComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUICascadeComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(ULineComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UTextComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UChildWidgetActorComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(URectTransformComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UButtonComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUIAnimationAttachComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UUIAnimationSlotComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UEventSystemComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UEventTriggerComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UCurveComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UFocusComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UInputFieldComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(USliderComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UScrollRectComponent::StaticClass()->GetFName());
	PropertyModule.UnregisterCustomClassLayout(UScrollbarComponent::StaticClass()->GetFName());
	
	UnregisterSubComponentClassLayout(UCanvasScalerSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UAspectRatioFitterSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UContentSizeFitterSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UOutlineSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UPositionAsUV1SubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UShadowSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UStandaloneInputModuleSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(URectMask2DSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(ULayoutElementSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UCanvasSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UCanvasRendererSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(URawImageSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UImageSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UBackgroundBlurSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UUIStaticMeshSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UUISimpleStaticMeshSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UUINiagaraSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UUICascadeSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(ULineSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UTextSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UHorizontalLayoutGroupSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UVerticalLayoutGroupSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UGridLayoutGroupSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UTextHypertextClickSubComponent::StaticClass()->GetFName());
	UnregisterSubComponentClassLayout(UFocusSubComponent::StaticClass()->GetFName());
	
	ISequencerModule& SequencerModule = FModuleManager::Get().GetModuleChecked<ISequencerModule>("Sequencer");

	// unregister specialty track editors
	SequencerModule.UnRegisterTrackEditor(ComponentMaterialTrackCreateEditorHandle);
	SequencerModule.UnRegisterTrackEditor(RotatorPropertyTrackCreateEditorHandle);
}

void FUGUIEditorModule::RegisterSubComponentClassLayout(FName ClassName,
	FOnGetSubComponentDetailCustomizationInstance DetailLayoutDelegate)
{
	if (ClassName != NAME_None)
	{
		FSubComponentDetailLayoutCallback Callback;
		Callback.DetailLayoutDelegate = DetailLayoutDelegate;
		ClassNameToDetailLayoutNameMap.Add(ClassName, Callback);
	}
}

void FUGUIEditorModule::UnregisterSubComponentClassLayout(FName ClassName)
{
	if (ClassName.IsValid() && (ClassName != NAME_None))
	{
		ClassNameToDetailLayoutNameMap.Remove(ClassName);
	}
}

bool FUGUIEditorModule::GetSubComponentClassLayout(FName ClassName,
	FOnGetSubComponentDetailCustomizationInstance& DetailLayoutDelegate)
{
	const auto DetailLayoutPtr = ClassNameToDetailLayoutNameMap.Find(ClassName);
	if (DetailLayoutPtr)
	{
		DetailLayoutDelegate = DetailLayoutPtr->DetailLayoutDelegate;
		return true;
	}
	return false;
}

void FUGUIEditorModule::OnPostEngineInit() const
{
	if (ISettingsModule* SettingModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingModule->RegisterSettings("Project", "Plugins", "UGUI",
			LOCTEXT("RuntimeSettingsName", "UGUI"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the UGUI plugin"),
			GetMutableDefault<UUGUISettings>()
		);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUGUIEditorModule, UGUIEditor)
