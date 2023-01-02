#include "Designer/Designer2DWidgetActor.h"
#include "Core/Layout/CanvasScalerSubComponent.h"
#include "UObject/ConstructorHelpers.h"

/////////////////////////////////////////////////////
// ADesigner2DWidgetActor

ADesigner2DWidgetActor::ADesigner2DWidgetActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UIRoot = CreateDefaultSubobject<URectTransformComponent>(TEXT("UIRoot"));
	RootComponent = UIRoot;

	UIRoot->SetSizeDelta(FVector2D(1920, 1080));
	UIRoot->AddSubComponentByClass(UCanvasSubComponent::StaticClass());
	UIRoot->AddSubComponentByClass(UCanvasScalerSubComponent::StaticClass());
	
	UCanvasSubComponent* CanvasComp = Cast<UCanvasSubComponent>(UIRoot->GetComponent(UCanvasSubComponent::StaticClass(), true));
	if (IsValid(CanvasComp))
	{
		CanvasComp->SetSortingOrder(1 + 1e8);
		CanvasComp->SetRenderMode(ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay);
		CanvasComp->bForceUseSelfRenderMode = true;
	}

	PrimitiveSelectComponent = CreateEditorOnlyDefaultSubobject<UUIPrimitiveSelectComponent>(TEXT("UIPrimitiveSelect"));
	if (PrimitiveSelectComponent)
	{
		PrimitiveSelectComponent->SetupAttachment(RootComponent);
	}

	SelectedRectDrawerComponent = CreateEditorOnlyDefaultSubobject<UUISelectedRectDrawerComponent>(TEXT("UISelectedRectDrawer"));
	if (SelectedRectDrawerComponent)
	{
		SelectedRectDrawerComponent->SetupAttachment(PrimitiveSelectComponent);
	}

	CheckBoxDrawComponent = CreateEditorOnlyDefaultSubobject<UImageComponent>(TEXT("UImageComponent"));
	if(CheckBoxDrawComponent)
	{
		CheckBoxDrawComponent->SetupAttachment(PrimitiveSelectComponent);
		CheckBoxDrawComponent->SetHidePrimitive(true);
		CheckBoxDrawComponent->SetAllDirty();

	    static ConstructorHelpers::FObjectFinderOptional<UPaperSprite> CheckBoxSprite_Finder(TEXT("PaperSprite'/UGUI/DefaultResources/Editor/CheckBox.CheckBox'"));
		CheckBoxDrawComponent->SetSprite(CheckBoxSprite_Finder.Get());
		CheckBoxDrawComponent->SetColor(FColor::FromHex("#091D354D").ReinterpretAsLinear());
		CheckBoxDrawComponent->SetImageType(EImageFillType::ImageFillType_Sliced);
		CheckBoxDrawComponent->SetBorder(FUIMargin(0.1, 0.1, 0.1, 0.1));
	}

	PrimitiveSelectComponent->SelectedRectDrawerComponent = SelectedRectDrawerComponent;
	PrimitiveSelectComponent->CheckBoxDrawComponent = CheckBoxDrawComponent;

	EventSystemComponent= CreateEditorOnlyDefaultSubobject<UEventSystemComponent>(TEXT("EventSystemComponent"));
	if (EventSystemComponent)
	{
#if WITH_EDITOR
		EventSystemComponent->bRegisterInEditor = true;
#endif
		EventSystemComponent->SetupAttachment(RootComponent);
	}
}

/////////////////////////////////////////////////////
