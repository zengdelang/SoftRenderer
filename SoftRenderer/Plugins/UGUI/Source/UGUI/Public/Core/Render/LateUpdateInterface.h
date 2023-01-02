#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LateUpdateInterface.generated.h"

UINTERFACE(BlueprintType)
class UGUI_API ULateUpdateInterface : public UInterface
{
	GENERATED_BODY()
};

class UGUI_API ILateUpdateInterface
{
	GENERATED_BODY()

public:
	virtual void LateUpdate() = 0;
	
};
