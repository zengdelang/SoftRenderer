#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LayoutIgnorerInterface.generated.h"

UINTERFACE(BlueprintType)
class UGUI_API ULayoutIgnorerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * A RectTransform will be ignored by the layout system if it has a component which implements ILayoutIgnorer.
 *
 * A components that implements ILayoutIgnorer can be used to make a parent layout group component not consider this RectTransform part of the group. The RectTransform can then be manually positioned despite being a child GameObject of a layout group.
 */
class UGUI_API ILayoutIgnorerInterface
{
	GENERATED_BODY()

public:
	/**
	 *  Should this RectTransform be ignored bvy the layout system?
	 *
	 *  Setting this property to true will make a parent layout group component not consider this RectTransform part of the group. The RectTransform can then be manually positioned despite being a child GameObject of a layout group.
	 */
	virtual bool IgnoreLayout() = 0;
	
};
