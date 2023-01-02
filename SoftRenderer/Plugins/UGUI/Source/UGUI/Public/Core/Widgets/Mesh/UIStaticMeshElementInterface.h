#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UIStaticMeshElementInterface.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API UUIStaticMeshElementInterface : public UInterface
{
    GENERATED_BODY()
};

class UGUI_API IUIStaticMeshElementInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UI Static Mesh")
    virtual UStaticMesh* GetStaticMesh() const { return nullptr; }

    UFUNCTION(BlueprintCallable, Category = "UI Static Mesh")
    virtual void SetStaticMesh(UStaticMesh* InStaticMesh) {}
	
};
