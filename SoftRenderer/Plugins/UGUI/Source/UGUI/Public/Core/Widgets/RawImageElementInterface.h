#pragma once

#include "CoreMinimal.h"
#include "Core/UICommonDefinitions.h"
#include "UObject/Interface.h"
#include "RawImageElementInterface.generated.h"

UENUM(BlueprintType)
enum class ERawImageMaskMode : uint8
{
	MaskMode_None UMETA(DisplayName = "None"),
	MaskMode_Circle UMETA(DisplayName = "Circle"),
	MaskMode_CircleRing UMETA(DisplayName = "CircleRing"),
};

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API URawImageElementInterface : public UInterface
{
	GENERATED_BODY()
};

class UGUI_API IRawImageElementInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = RawImage)
	virtual UTexture* GetTexture() const { return nullptr; }

	UFUNCTION(BlueprintCallable, Category = RawImage)
	virtual void SetTexture(UTexture* InTexture) {}

	UFUNCTION(BlueprintCallable, Category = RawImage)
	virtual FUVRect GetUVRect() const { return FUVRect(); }

	UFUNCTION(BlueprintCallable, Category = RawImage)
	virtual void SetUVRect(FUVRect InUVRect) {}

	UFUNCTION(BlueprintCallable, Category = RawImage)
    virtual bool GetPreserveAspect() const { return false; }
 
	UFUNCTION(BlueprintCallable, Category = RawImage)
	virtual void SetPreserveAspect(bool bInPreserveAspect) {}

	UFUNCTION(BlueprintCallable, Category = RawImage)
	virtual bool GetFillViewRect() const { return false; }

	UFUNCTION(BlueprintCallable, Category = RawImage)
	virtual void SetFillViewRect(bool bInFillViewRect) {}

	UFUNCTION(BlueprintCallable, Category = Image)
	virtual ERawImageMaskMode GetImageMaskMode() const { return ERawImageMaskMode::MaskMode_None; }

	UFUNCTION(BlueprintCallable, Category = Image)
	virtual void SetImageMaskMode(ERawImageMaskMode InImageMaskType) {}

	UFUNCTION(BlueprintCallable, Category = Image)
	virtual float GetThickness() const { return 0;}

	UFUNCTION(BlueprintCallable, Category = Image)
	virtual void SetThickness(float InThickness) {}

};
