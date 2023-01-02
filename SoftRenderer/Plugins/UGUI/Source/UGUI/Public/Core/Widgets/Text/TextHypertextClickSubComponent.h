#pragma once

#include "CoreMinimal.h"
#include "TextElementInterface.h"
#include "EventSystem/EventData/PointerEventData.h"
#include "EventSystem/Interfaces/PointerClickHandlerInterface.h"
#include "EventSystem/Interfaces/PointerDownHandlerInterface.h"
#include "TextHypertextClickSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "TextHypertextClick"))
class UGUI_API UTextHypertextClickSubComponent : public UBehaviourSubComponent, public IPointerClickHandlerInterface, public IPointerDownHandlerInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(Transient)
	TScriptInterface<ITextElementInterface> TextElement;
	
public:
	virtual void OnPointerClick(UPointerEventData* EventData) override;
	virtual void OnPointerDown(UPointerEventData* EventData) override;
	
};
