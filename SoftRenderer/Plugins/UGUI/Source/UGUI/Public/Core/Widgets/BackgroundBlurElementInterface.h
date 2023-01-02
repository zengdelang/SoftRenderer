#pragma once

#include "CoreMinimal.h"
#include "Core/UICommonDefinitions.h"
#include "UObject/Interface.h"
#include "BackgroundBlurElementInterface.generated.h"

UENUM(BlueprintType)
enum class EBackgroundBlurMaskMode : uint8
{
	MaskMode_None UMETA(DisplayName = "None"),

	MaskMode_Circle UMETA(DisplayName = "Circle"),
};

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API UBackgroundBlurElementInterface : public UInterface
{
	GENERATED_BODY()
};

class UGUI_API IBackgroundBlurElementInterface
{
	GENERATED_BODY()

protected:
	TSharedPtr<FUIBlurGraphicData> GraphicData;

public:
	UFUNCTION(BlueprintCallable, Category = BackgroundBlur)
	virtual float GetBlurStrength() { return 0; }

	UFUNCTION(BlueprintCallable, Category = BackgroundBlur)
	virtual void SetBlurStrength(float InBlurStrength) {}

	UFUNCTION(BlueprintCallable, Category = BackgroundBlur)
	virtual int32 GetBlurRadius() { return 0; }

	UFUNCTION(BlueprintCallable, Category = BackgroundBlur)
	virtual void SetBlurRadius(int32 InBlurRadius) {}

	UFUNCTION(BlueprintCallable, Category = BackgroundBlur)
	virtual bool IsApplyAlphaToBlur() { return true; }

	UFUNCTION(BlueprintCallable, Category = BackgroundBlur)
	virtual void SetApplyAlphaToBlur(bool bInApplyAlphaToBlur) {}

	UFUNCTION(BlueprintCallable, Category = BackgroundBlur)
	virtual bool IsOverrideAutoRadiusCalculation() { return false; }

	UFUNCTION(BlueprintCallable, Category = BackgroundBlur)
	virtual void SetOverrideAutoRadiusCalculation(bool bInOverrideAutoRadiusCalculation) {}

	UFUNCTION(BlueprintCallable, Category = BackgroundBlur)
	virtual UTexture* GetMaskTexture() { return nullptr; };

	UFUNCTION(BlueprintCallable, Category = BackgroundBlur)
	virtual void SetMaskTexture(UTexture2D* InMaskTexture) {}

};
