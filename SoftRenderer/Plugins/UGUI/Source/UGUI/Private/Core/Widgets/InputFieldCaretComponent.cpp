#include "Core/Widgets/InputFieldCaretComponent.h"
#include "Core/Layout/LayoutElementSubComponent.h"
#include "Core/Render/CanvasRendererSubComponent.h"

/////////////////////////////////////////////////////
// UInputFieldCaretComponent

UInputFieldCaretComponent::UInputFieldCaretComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CanvasRenderer = CreateDefaultSubobject<UCanvasRendererSubComponent>(TEXT("CanvasRendererSubComp0"));
	if (CanvasRenderer)
	{
		SubComponents.Emplace(CanvasRenderer);
	}

	const auto LayoutElementSubComponent = CreateDefaultSubobject<ULayoutElementSubComponent>(TEXT("LayoutElementSubComp0"));
	if (LayoutElementSubComponent)
	{
		LayoutElementSubComponent->SetIgnoreLayout(true);
		SubComponents.Emplace(LayoutElementSubComponent);
	}
}

/////////////////////////////////////////////////////
