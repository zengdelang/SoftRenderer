#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/MaskableGraphicComponent.h"
#include "UISimpleStaticMeshElementInterface.h"
#include "Core/Layout/LayoutElementInterface.h"
#include "UISimpleStaticMeshComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Mesh), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent, DisplayName = "UI Simple Static Mesh"))
class UGUI_API UUISimpleStaticMeshComponent : public UMaskableGraphicComponent, public ILayoutElementInterface, public IUISimpleStaticMeshElementInterface
{
	GENERATED_UCLASS_BODY()

protected:
	/** The static mesh that this component uses to render */
	UPROPERTY(EditAnywhere, Category = Graphic)
	UStaticMesh* StaticMesh;

    UPROPERTY(EditAnywhere, Category = Graphic)
    UTexture* Texture;
	
    UPROPERTY(EditAnywhere, Category = Graphic, meta = (DisplayName = "Relative Location"))
    FVector MeshRelativeLocation;

    UPROPERTY(EditAnywhere, Category = Graphic, meta = (DisplayName = "Relative Rotation"))
    FRotator MeshRelativeRotation;

    UPROPERTY(EditAnywhere, Category = Graphic, meta = (DisplayName = "Relative Scale 3D"))
    FVector MeshRelativeScale3D;

    UPROPERTY(EditAnywhere, Category = Graphic)
    uint8 bAlignBoundingBoxCenter : 1;

    /** Cache that avoids Quat<->Rotator conversions if possible. Only to be used with RelativeRotation. */
    FRotationConversionCache RelativeRotationCache;

protected:
    virtual UMaterialInterface* GetMaterial() const override;
	
	virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override;

public:
    //~ Begin ILayoutElementInterface Interface
    virtual void CalculateLayoutInputHorizontal() override {};
    virtual void CalculateLayoutInputVertical() override {};
    virtual float GetMinWidth() override { return 0; };
    virtual float GetPreferredWidth() override;
    virtual float GetFlexibleWidth() override { return -1; };
    virtual float GetMinHeight() override { return 0; };
    virtual float GetPreferredHeight() override;
    virtual float GetFlexibleHeight() override { return -1; };
    virtual int32 GetLayoutPriority() override { return 0; };
    //~ End ILayoutElementInterface Interface

    //~ Begin IUIPrimitiveElementInterface Interface
    virtual int32 GetNumOverrideMaterials() const override;
    virtual UMaterialInterface* GetOverrideMaterial(int32 MaterialIndex) const override;
    //~ End IUIPrimitiveElementInterface Interface
	
public:
    virtual UTexture* GetMainTexture() const override
    {
        return Texture;
    }

public:
	virtual UStaticMesh* GetStaticMesh() const override
	{
		return StaticMesh;
	}

    virtual void SetStaticMesh(UStaticMesh* InStaticMesh) override
    {
        if (IsValid(StaticMesh))
        {
            if (StaticMesh != InStaticMesh)
            {
                const bool bSkipLayoutUpdate = IsValid(InStaticMesh) ? InStaticMesh->GetBoundingBox() == StaticMesh->GetBoundingBox() : false;
                const bool bSkipMaterialUpdate = IsValid(InStaticMesh) ? InStaticMesh ->GetMaterial(0) == StaticMesh->GetMaterial(0) : false;

                StaticMesh = InStaticMesh;

                if (!bSkipLayoutUpdate)
                    SetLayoutDirty();

                if (!bSkipMaterialUpdate)
                    SetMaterialDirty();

                SetVerticesDirty();
            }
        }
        else if (IsValid(InStaticMesh))
        {
            StaticMesh = InStaticMesh;
            SetAllDirty();
        }
    }

    virtual UTexture* GetTexture() const override
    {
        return Texture;
    }

    virtual void SetTexture(UTexture* InTexture) override
    {
        if (Texture != InTexture)
        {
            Texture = InTexture;
            SetMaterialDirty();
        }
    }

	virtual FVector GetMeshRelativeLocation() const override
	{
        return MeshRelativeLocation;
	}

    virtual void SetMeshRelativeLocation(FVector InRelativeLocation) override
	{
		if (MeshRelativeLocation != InRelativeLocation)
		{
            MeshRelativeLocation = InRelativeLocation;
            SetVerticesDirty();
		}
	}

    virtual FRotator GetMeshRelativeRotation() const override
    {
        return MeshRelativeRotation;
    }

    virtual void SetMeshRelativeRotation(FRotator InRelativeRotation) override
    {
        if (MeshRelativeRotation != InRelativeRotation)
        {
            MeshRelativeRotation = InRelativeRotation;
            SetLayoutDirty();
        	SetVerticesDirty();
        }
    }

    virtual FVector GetMeshRelativeScale3D() const override
    {
        return MeshRelativeScale3D;
    }

    virtual void SetMeshRelativeScale3D(FVector InRelativeScale3D) override
    {
        if (MeshRelativeScale3D != InRelativeScale3D)
        {
            MeshRelativeScale3D = InRelativeScale3D;
            SetLayoutDirty();
        	SetVerticesDirty();
        }
    }

    virtual bool GetAlignBoundingBoxCenter() const override
    {
        return bAlignBoundingBoxCenter;
    }

    virtual void SetAlignBoundingBoxCenter(bool bInAlignBoundingBoxCenter) override
    {
        if (bAlignBoundingBoxCenter != bInAlignBoundingBoxCenter)
        {
            bAlignBoundingBoxCenter = bInAlignBoundingBoxCenter;
            SetVerticesDirty();
        }
    }
	
};
