#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UIPrimitiveElementInterface.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API UUIPrimitiveElementInterface : public UInterface
{
    GENERATED_BODY()
};

class UGUI_API IUIPrimitiveElementInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UI Primitive")
    virtual int32 GetNumOverrideMaterials() const { return 0; };

    UFUNCTION(BlueprintCallable, Category = "UI Primitive")
    virtual UMaterialInterface* GetOverrideMaterial(int32 MaterialIndex) const { return nullptr; }

    UFUNCTION(BlueprintCallable, Category = "UI Primitive")
    virtual void SetOverrideMaterial(int32 ElementIndex, UMaterialInterface* InMaterial) {}
	
};
