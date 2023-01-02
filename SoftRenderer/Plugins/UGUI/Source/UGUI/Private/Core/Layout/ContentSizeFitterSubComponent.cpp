#include "Core/Layout/ContentSizeFitterSubComponent.h"
#include "Core/Layout/LayoutGroupInterface.h"
#include "Core/Layout/LayoutUtility.h"
#include "UGUI.h"

#if WITH_EDITORONLY_DATA && WITH_EDITOR
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Logging/TokenizedMessage.h"
#endif

/////////////////////////////////////////////////////
// UContentSizeFitterSubComponent

UContentSizeFitterSubComponent::UContentSizeFitterSubComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , HorizontalFit(EFitMode::FitMode_Unconstrained)
    , VerticalFit(EFitMode::FitMode_Unconstrained)
    , bHandlingSelfFitting(false)
{
    
}

#if WITH_EDITORONLY_DATA

void UContentSizeFitterSubComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    SetDirty();
}

void UContentSizeFitterSubComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
    SetDirty();
}

#endif

void UContentSizeFitterSubComponent::OnRectTransformDimensionsChange()
{
    if (!bHandlingSelfFitting)
    {
        SetDirty();
    }
}

void UContentSizeFitterSubComponent::HandleSelfFittingAlongAxis(int32 Axis)
{
	if (!IsValid(AttachTransform))
	{
		return;
	}

	bool bCanPerformLayout = true;
	UBehaviourComponent* SceneParent = Cast<UBehaviourComponent>(AttachTransform->GetAttachParent());
	if (IsValid(SceneParent))
	{
		const TScriptInterface<ILayoutGroupInterface> LayoutGroup = SceneParent->GetComponentByInterface(ULayoutGroupInterface::StaticClass(), false);
		if (LayoutGroup)
		{
			bCanPerformLayout = false;  
		}
	}
	
	if (!bCanPerformLayout)
	{
		if (IsValid(AttachTransform) && IsValid(AttachTransform->GetOwner()))
		{
			UE_LOG(LogUGUI, Warning, TEXT("Parent has a type of layout group component. A child of a layout group should not have a Content Size Fitter component, since it should be driven by the layout group. WidgetAcotr : %s"), *AttachTransform->GetOwner()->GetFullName());

#if WITH_EDITORONLY_DATA && WITH_EDITOR
			FNotificationInfo Info(FText::FromString(TEXT("There is an invalid ContentSizeFitter component, Open the messagelog tab to view the details.")));
			Info.Image = FEditorStyle::GetBrush(FTokenizedMessage::GetSeverityIconName(EMessageSeverity::Warning));
			Info.bFireAndForget = true;
			Info.bUseThrobber = false;
			Info.ExpireDuration = 4.f;
			FSlateNotificationManager::Get().AddNotification(Info);
#endif

			SetEnabled(false);
		}
		return;	
	}
	
    const EFitMode Fitting = (Axis == 0 ? HorizontalFit : VerticalFit);
    if (Fitting == EFitMode::FitMode_Unconstrained)
    {
        return;
    }

    const auto RectTransform = Cast<URectTransformComponent>(GetOuter());
    if (!IsValid(RectTransform))
        return;

    bHandlingSelfFitting = true;
	
    // Set size to min or preferred size
    if (Fitting == EFitMode::FitMode_MinSize)
    {
        RectTransform->SetSizeWithCurrentAnchors(static_cast<ERectTransformAxis>(Axis), FLayoutUtility::GetMinSize(RectTransform, Axis));
    }
    else
    {
        RectTransform->SetSizeWithCurrentAnchors(static_cast<ERectTransformAxis>(Axis), FLayoutUtility::GetPreferredSize(RectTransform, Axis));
    }

    bHandlingSelfFitting = false;
}

/////////////////////////////////////////////////////
