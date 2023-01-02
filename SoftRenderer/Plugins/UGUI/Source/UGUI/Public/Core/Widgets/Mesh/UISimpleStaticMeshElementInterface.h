#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UISimpleStaticMeshElementInterface.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API UUISimpleStaticMeshElementInterface : public UInterface
{
    GENERATED_BODY()
};

class UGUI_API IUISimpleStaticMeshElementInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UI Simpale Static Mesh")
    virtual UStaticMesh* GetStaticMesh() const { return nullptr; }

    UFUNCTION(BlueprintCallable, Category = "UI Simpale Static Mesh")
    virtual void SetStaticMesh(UStaticMesh* InStaticMesh) {}

    UFUNCTION(BlueprintCallable, Category = "UI Simpale Static Mesh")
    virtual UTexture* GetTexture() const { return nullptr; }

    UFUNCTION(BlueprintCallable, Category = "UI Simpale Static Mesh")
    virtual void SetTexture(UTexture* InTexture) {}

    UFUNCTION(BlueprintCallable, Category = "UI Simpale Static Mesh")
    virtual FVector GetMeshRelativeLocation() const { return FVector::ZeroVector; }

    UFUNCTION(BlueprintCallable, Category = "UI Simpale Static Mesh")
    virtual void SetMeshRelativeLocation(FVector InRelativeLocation){}

    UFUNCTION(BlueprintCallable, Category = "UI Simpale Static Mesh")
    virtual FRotator GetMeshRelativeRotation() const { return FRotator::ZeroRotator; }

    UFUNCTION(BlueprintCallable, Category = "UI Simpale Static Mesh")
    virtual void SetMeshRelativeRotation(FRotator InRelativeRotation) {}
	
    UFUNCTION(BlueprintCallable, Category = "UI Simpale Static Mesh")
    virtual FVector GetMeshRelativeScale3D() const { return FVector::ZeroVector; }

    UFUNCTION(BlueprintCallable, Category = "UI Simpale Static Mesh")
    virtual void SetMeshRelativeScale3D(FVector InRelativeScale3D) {}

    UFUNCTION(BlueprintCallable, Category = "UI Simpale Static Mesh")
    virtual bool GetAlignBoundingBoxCenter() const { return false; }

    UFUNCTION(BlueprintCallable, Category = "UI Simpale Static Mesh")
    virtual void SetAlignBoundingBoxCenter(bool bInAlignBoundingBoxCenter) {}
	
};
