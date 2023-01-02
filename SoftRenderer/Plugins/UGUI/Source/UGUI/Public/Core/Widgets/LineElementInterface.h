#pragma once

#include "CoreMinimal.h"
#include "Core/GeometryUtility.h"
#include "UObject/Interface.h"
#include "LineElementInterface.generated.h"

struct FRect;

UENUM(BlueprintType)
enum class ELineType : uint8
{
	None,
	RectBox,
	RectLeftAndRight,
	RectTopAndBottom,
	RectLeft,
	RectTop,
	RectRight,
	RectBottom,
	CustomPoints,
	RectCustomPoints,
};

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API ULineElementInterface : public UInterface
{
	GENERATED_BODY()
};

class UGUI_API ILineElementInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Line)
	virtual FLinearColor GetLineColor() const { return FLinearColor::White; }
	
	UFUNCTION(BlueprintCallable, Category = Line)
	virtual void SetLineColor(FLinearColor InLineColor) {}
	
	UFUNCTION(BlueprintCallable, Category = Line)
	virtual bool GetClosedLine() const { return false; }
	
	UFUNCTION(BlueprintCallable, Category = Line)
	virtual void SetClosedLine(bool bInClosedLine) {}
	
	UFUNCTION(BlueprintCallable, Category = Line)
	virtual float GetLineThickness() const { return 0;}

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual void SetLineThickness(float InThickness) {}

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual FLinearColor GetPointColor() { return FLinearColor::White; }

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual void SetPointColor(FLinearColor InPointColor) {}

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual float GetPointSize() { return 0; }

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual void SetPointSize(float InPointSize) {}

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual EPointMode GetPointMode() { return EPointMode::None;}

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual void SetPointMode(EPointMode InPointMode) {}

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual TArray<FVector2D>& GetLinePoints()
	{
		static TArray<FVector2D> EmptyPoints;
		return EmptyPoints;
	}

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual void RedrawLine() {}
	
	UFUNCTION(BlueprintCallable, Category = Line)
	virtual void AddPoint(FVector2D InPoint) {}

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual bool RemovePoint(int32 InPointIndex) { return false; }

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual void ClearPoints() {}
	
	UFUNCTION(BlueprintCallable, Category = Line)
	virtual ELineType GetLineType() { return ELineType::None; }
	
	UFUNCTION(BlueprintCallable, Category = Line)
	virtual void SetLineType(ELineType InLineType) {}

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual float GetDrawLineTolerance() { return 0.1f; }
	
	UFUNCTION(BlueprintCallable, Category = Line)
	virtual void SetDrawLineTolerance(float InDrawLineTolerance) {}

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual UObject* GetSprite() const { return nullptr; }

	UFUNCTION(BlueprintCallable, Category = Line)
	virtual void SetSprite(UObject* InSprite) {}

protected:
	static TArray<FVector2D> RectPoints;

	virtual FRect GetTransformRect() = 0;
	void GenerateRectPoints(ELineType LineType, bool bClosedLine);
	
};
