#pragma once

#include "CoreMinimal.h"
#include "Core/BehaviourComponent.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Interfaces/BeginDragHandlerInterface.h"
#include "Interfaces/CancelHandlerInterface.h"
#include "Interfaces/DeselectHandlerInterface.h"
#include "Interfaces/DragHandlerInterface.h"
#include "Interfaces/DropHandlerInterface.h"
#include "Interfaces/EndDragHandlerInterface.h"
#include "Interfaces/InitializePotentialDragHandlerInterface.h"
#include "Interfaces/MoveHandlerInterface.h"
#include "Interfaces/PointerClickHandlerInterface.h"
#include "Interfaces/PointerDownHandlerInterface.h"
#include "Interfaces/PointerEnterHandlerInterface.h"
#include "Interfaces/PointerExitHandlerInterface.h"
#include "Interfaces/PointerUpHandlerInterface.h"
#include "Interfaces/ScrollHandlerInterface.h"
#include "Interfaces/SelectHandlerInterface.h"
#include "Interfaces/SubmitHandlerInterface.h"
#include "Interfaces/UpdateSelectedHandlerInterface.h"
#include "EventTriggerComponent.generated.h"
 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FETPointerEventDelegate, UPointerEventData*, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FETBaseEventDelegate, UBaseEventData*, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FETAxisEventDelegate, UAxisEventData*, EventData);

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Event), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UEventTriggerComponent : public URectTransformComponent,
    public IPointerEnterHandlerInterface,
    public IPointerExitHandlerInterface,
    public IPointerDownHandlerInterface,
    public IPointerUpHandlerInterface,
    public IPointerClickHandlerInterface,
    public IInitializePotentialDragHandlerInterface,
    public IBeginDragHandlerInterface,
    public IDragHandlerInterface,
    public IEndDragHandlerInterface,
    public IDropHandlerInterface,
    public IScrollHandlerInterface,
    public IUpdateSelectedHandlerInterface,
    public ISelectHandlerInterface,
    public IDeselectHandlerInterface,
    public IMoveHandlerInterface,
    public ISubmitHandlerInterface,
    public ICancelHandlerInterface
{
	GENERATED_UCLASS_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnPointerEnter))
    FETPointerEventDelegate OnPointerEnterDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnPointerExit))
    FETPointerEventDelegate OnPointerExitDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnPointerDown))
    FETPointerEventDelegate OnPointerDownDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnPointerUp))
    FETPointerEventDelegate OnPointerUpDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnPointerClick))
    FETPointerEventDelegate OnPointerClickDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnInitializePotentialDrag))
    FETPointerEventDelegate OnInitializePotentialDragDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnBeginDrag))
    FETPointerEventDelegate OnBeginDragDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnDrag))
    FETPointerEventDelegate OnDragDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnEndDrag))
    FETPointerEventDelegate OnEndDragDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnDrop))
    FETPointerEventDelegate OnDropDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnScroll))
    FETPointerEventDelegate OnScrollDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnUpdateSelected))
    FETBaseEventDelegate OnUpdateSelectedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnSelect))
    FETBaseEventDelegate OnSelectDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnDeselect))
    FETBaseEventDelegate OnDeselectDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnMove))
    FETAxisEventDelegate OnMoveDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnSubmit))
    FETBaseEventDelegate OnSubmitDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Components|Input", meta = (DisplayName = OnCancel))
    FETBaseEventDelegate OnCancelDelegate;
	
public:
    virtual void OnPointerEnter(UPointerEventData* EventData) override;
    virtual void OnPointerExit(UPointerEventData* EventData) override;
    virtual void OnPointerDown(UPointerEventData* EventData) override;
    virtual void OnPointerUp(UPointerEventData* EventData) override;
    virtual void OnPointerClick(UPointerEventData* EventData) override;
    virtual void OnInitializePotentialDrag(UPointerEventData* EventData) override;
    virtual void OnBeginDrag(UPointerEventData* EventData) override;
    virtual void OnDrag(UPointerEventData* EventData) override;
    virtual void OnEndDrag(UPointerEventData* EventData) override;
    virtual void OnDrop(UPointerEventData* EventData) override;
    virtual void OnScroll(UPointerEventData* EventData) override;
    virtual void OnUpdateSelected(UBaseEventData* EventData) override;
    virtual void OnSelect(UBaseEventData* EventData) override;
    virtual void OnDeselect(UBaseEventData* EventData) override;
    virtual void OnMove(UAxisEventData* EventData) override;
    virtual void OnSubmit(UBaseEventData* EventData) override;
    virtual void OnCancel(UBaseEventData* EventData) override;
	
};
