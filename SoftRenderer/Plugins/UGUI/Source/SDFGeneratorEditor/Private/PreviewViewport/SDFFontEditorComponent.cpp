#include "SDFFontEditorComponent.h"

/////////////////////////////////////////////////////
// USDFFontEditorComponent

USDFFontEditorComponent::USDFFontEditorComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer), TextComponent(nullptr), ViewportInfo(nullptr)
{
	bTickInEditor = true;
	PrimaryComponentTick.bHighPriority = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
}

void USDFFontEditorComponent::Awake()
{
    Super::Awake();
	
    TextComponent = NewObject<UTextComponent>(this);
    TextComponent->RegisterComponent();
	TextComponent->SetFontSize(150);
	TextComponent->SetHorizontalOverflow(EHorizontalWrapMode::HorizontalWrapMode_Overflow);
	TextComponent->SetVerticalOverflow(EVerticalWrapMode::VerticalWrapMode_Overflow);
	TextComponent->SetAlignment(ETextAnchor::TextAnchor_MiddleCenter);
    TextComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
    TextComponent->SetLocalTransform(FTransform::Identity);
    TextComponent->AwakeFromLoad();
	
    if (!IsValid(ViewportInfo))
    {
        UWorld* World = GetWorld();
        check(World);

        for (int32 Index = 0, Count = World->ExtraReferencedObjects.Num(); Index < Count; ++Index)
        {
            const auto& Object = Cast<USDFFontEditorViewportInfo>(World->ExtraReferencedObjects[Index]);
            if (Object)
            {
                ViewportInfo = Object;
                break;
            }
        }
    }

	if (IsValid(ViewportInfo) && ViewportInfo->SDFFontEditor.IsValid())
	{
		TextComponent->SetFont(ViewportInfo->SDFFontEditor.Pin()->GetSDFFont());
	}
}

void USDFFontEditorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsValid(ViewportInfo) && ViewportInfo->SDFFontEditor.IsValid())
	{
		if (ViewportInfo->SDFFontEditor.Pin()->IsRefreshTextComponent())
		{
			TextComponent->DestroyComponent();

			TextComponent = NewObject<UTextComponent>(this);
			TextComponent->RegisterComponent();
			TextComponent->SetFontSize(150);
			TextComponent->SetHorizontalOverflow(EHorizontalWrapMode::HorizontalWrapMode_Overflow);
			TextComponent->SetVerticalOverflow(EVerticalWrapMode::VerticalWrapMode_Overflow);
			TextComponent->SetAlignment(ETextAnchor::TextAnchor_MiddleCenter);
			TextComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
			TextComponent->SetLocalTransform(FTransform::Identity);
			TextComponent->AwakeFromLoad();

			TextComponent->SetFont(ViewportInfo->SDFFontEditor.Pin()->GetSDFFont());
		}
		
		TextComponent->SetText(ViewportInfo->SDFFontEditor.Pin()->GetPreviewText());
	}
}

/////////////////////////////////////////////////////
