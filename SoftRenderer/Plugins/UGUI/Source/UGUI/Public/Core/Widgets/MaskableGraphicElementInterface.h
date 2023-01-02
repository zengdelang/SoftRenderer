#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MaskableGraphicElementInterface.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FMaskableGraphicCullStateChangedEvent, bool);

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API UMaskableGraphicElementInterface : public UInterface
{
	GENERATED_BODY()
};

class UGUI_API IMaskableGraphicElementInterface
{
	GENERATED_BODY()

public:
	/**
	 * Callback issued when culling changes.
	 *
	 * Called when the culling state of this MaskableGraphic either becomes culled or visible. You can use this to control other elements of your UI as culling happens.
	 */
	FMaskableGraphicCullStateChangedEvent OnCullStateChanged;

public:
	UFUNCTION(BlueprintCallable, Category = MaskableGraphic)
	virtual	bool IsMaskable() const { return true; }

	UFUNCTION(BlueprintCallable, Category = MaskableGraphic)
	virtual void SetMaskable(bool bInMaskable) {}

};
