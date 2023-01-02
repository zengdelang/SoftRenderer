#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

class FUGUIEditorStyle : public FSlateStyleSet
{
public:
	FUGUIEditorStyle() : FSlateStyleSet("UGUIEditorStyleStyle")
	{
		
	}

	~FUGUIEditorStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}

	void InitStyles()
	{
		const FVector2D Icon16x16(16.f, 16.f);
		const FVector2D Icon64x64(64.f, 64.f);
		const FVector2D Icon23x20(23.f, 20.f);
		const FVector2D Icon20x23(20.f, 23.f);
		
		SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("UGUI"))->GetBaseDir() / TEXT("Resources"));

		Set("UGUI.GameObject",  new FSlateImageBrush(RootToContentDir(TEXT("Icons/WidgetActor_64x64"), TEXT(".png")), Icon64x64));
		
		Set("UGUI.Triangle",  new FSlateImageBrush(RootToContentDir(TEXT("Icons/Triangle"), TEXT(".png")), Icon23x20));
		Set("UGUI.TriangleLeft", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Triangle@Left"), TEXT(".png")), Icon20x23));

		Set("ClassIcon.WidgetActor", new FSlateImageBrush(RootToContentDir(TEXT("Icons/WidgetActor_16x16"), TEXT(".png")), Icon16x16));
		Set("ClassThumbnail.WidgetActor", new FSlateImageBrush(RootToContentDir(TEXT("Icons/WidgetActor_64x64"), TEXT(".png")), Icon64x64));
		Set("ClassIcon.BehaviourSubComponent",new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		
		//Set UIComponent Icon
		Set("ClassIcon.BackgroundBlurComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.BackgroundGlitchComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.CanvasGroupComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/CanvasGroup"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ButtonComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Button"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ChildWidgetActorComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.DefaultRootComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Canvas"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.EventSystemComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/EventSystem"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.EventTriggerComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/EventTrigger"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.GridLayoutGroupComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/GridLayoutGroup"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.HorizontalLayoutGroupComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/HorizontalBox"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ImageComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Image"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.InputFieldComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/EditableText"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.LineComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.RawImageComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/RawImage"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.RectTransformComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/RectTransform"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.SafeZoneComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/SafeZone"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ScrollGridLayoutComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ScrollListLayoutComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ScrollRectComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/ScrollBox"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ScrollbarComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/ScrollBar"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.SliderComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Slider"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.TextComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/TextBlock"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ToggleComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/CheckBox"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ToggleGroupComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/ToggleGroup"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.UICascadeComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.UINiagaraComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.UISimpleStaticMeshComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.UIStaticMeshComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.VerticalLayoutGroupComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/VerticalBox"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.DropdownComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Dropdown@16"), TEXT(".png")), Icon16x16));
		
		//Set UISubComponent Icon
		Set("ClassIcon.GridLayoutGroupSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/GridLayoutGroup"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.CanvasSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Canvas"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.CanvasRendererSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/CanvasRenderer"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.CanvasScalerSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/CanvasScaler@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ContentSizeFilterSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/ContentSizeFitter@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.TextSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Text@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.TextHypertextClickSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Text@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ImageSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Image@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.RawImageSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/RawImage@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.HorizontalLayoutGroupSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/HorizontalLayoutGroup@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.VerticalLayoutGroupSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/VerticalLayoutGroup@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.LineSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.UIStaticMeshSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.UISimpleStaticMeshSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.UICascadeSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.UINiagaraSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.BackgroundBlurSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.BackgroundGlitchSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ContentSizeFitterSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/ContentSizeFitter@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.GraphicRaycasterSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/GraphicRaycaster@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.FocusSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Widget"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.LayoutElementSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/LayoutElement@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.OutlineSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Outline@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.PositionAsUV1SubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/PositionAsUV1@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.RectMask2DSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/RectMask2D@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.ShadowSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/Shadow@16"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.StandaloneInputModuleSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/StandaloneInputModule"), TEXT(".png")), Icon16x16));
		Set("ClassIcon.AspectRatioFitterSubComponent", new FSlateImageBrush(RootToContentDir(TEXT("Icons/AspectRatioFitter@16"), TEXT(".png")), Icon16x16));

		//Set TextComponent Icon
		Set("TextIcon.VAlignTop",new FSlateImageBrush(RootToContentDir(TEXT("Icons/VAlignTop"),TEXT(".png")),Icon16x16));
		Set("TextIcon.VAlignMiddle",new FSlateImageBrush(RootToContentDir(TEXT("Icons/VAlignMiddle"),TEXT(".png")),Icon16x16));
		Set("TextIcon.VAlignBottom",new FSlateImageBrush(RootToContentDir(TEXT("Icons/VAlignBottom"),TEXT(".png")),Icon16x16));
		Set("TextIcon.HAlignLeft",new FSlateImageBrush(RootToContentDir(TEXT("Icons/HAlignLeft"),TEXT(".png")),Icon16x16));
		Set("TextIcon.HAlignCenter",new FSlateImageBrush(RootToContentDir(TEXT("Icons/HAlignCenter"),TEXT(".png")),Icon16x16));
		Set("TextIcon.HAlignRight",new FSlateImageBrush(RootToContentDir(TEXT("Icons/HAlignRight"),TEXT(".png")),Icon16x16));

		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

public:
	static FUGUIEditorStyle& Get()
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
	static TOptional<FUGUIEditorStyle> Singleton;
	
};
