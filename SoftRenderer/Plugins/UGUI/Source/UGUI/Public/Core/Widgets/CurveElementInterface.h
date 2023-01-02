#pragma once

#include "CoreMinimal.h"
#include "Core/GeometryUtility.h"
#include "UObject/Interface.h"
#include "CurveElementInterface.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API UCurveElementInterface : public UInterface
{
	GENERATED_BODY()
};

class UGUI_API ICurveElementInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Curve)
	virtual FLinearColor GetLineColor() const { return FLinearColor::White; }
	
	UFUNCTION(BlueprintCallable, Category = Curve)
	virtual void SetLineColor(FLinearColor InLineColor) {}
	
	UFUNCTION(BlueprintCallable, Category = Curve)
	virtual float GetLineThickness() const { return 0;}

	UFUNCTION(BlueprintCallable, Category = Curve)
	virtual void SetLineThickness(float InThickness) {}

	UFUNCTION(BlueprintCallable, Category = Curve)
	virtual FLinearColor GetPointColor() { return FLinearColor::White; }

	UFUNCTION(BlueprintCallable, Category = Curve)
	virtual void SetPointColor(FLinearColor InPointColor) {}

	UFUNCTION(BlueprintCallable, Category = Curve)
	virtual float GetPointSize() { return 0; }

	UFUNCTION(BlueprintCallable, Category = Curve)
	virtual void SetPointSize(float InPointSize) {}

	UFUNCTION(BlueprintCallable, Category = Curve)
	virtual EPointMode GetPointMode() { return EPointMode::None;}

	UFUNCTION(BlueprintCallable, Category = Curve)
	virtual void SetPointMode(EPointMode InPointMode) {}

};
