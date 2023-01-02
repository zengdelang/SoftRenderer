#pragma once

#include "CoreMinimal.h"
#include "Core/GeometryUtility.h"
#include "CurveElementInterface.h"
#include "MaskableGraphicComponent.h"
#include "CurveComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (UI), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UCurveComponent : public UMaskableGraphicComponent, public ICurveElementInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = Graphic, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float LineThickness;

	UPROPERTY(EditAnywhere, Category = Graphic)
	FLinearColor PointColor;

	UPROPERTY(EditAnywhere, Category = Graphic, meta = (ClampMin = "0.1", UIMin = "0.1"))
	float PointSize;

	UPROPERTY(EditAnywhere, Category = Graphic)
	EPointMode PointMode;

	UPROPERTY(EditAnywhere, Category = Graphic)
	UCurveFloat* Curve;

protected:
	virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override;
	
public:
	virtual FLinearColor GetLineColor() const override
	{
		return Color;
	}
	
	virtual void SetLineColor(FLinearColor InLineColor) override
	{
		if (Color != InLineColor)
		{
			Color = InLineColor;
			SetVerticesDirty();
		}
	}
	
	virtual float GetLineThickness() const override
	{
		return LineThickness;
	}
	
	virtual void SetLineThickness(float InThickness) override
	{
		if (LineThickness != InThickness)
		{
			LineThickness = FMath::Max(0.01f, InThickness);
			SetVerticesDirty();
		}
	}
	
	virtual FLinearColor GetPointColor() override
	{
		return PointColor;
	}
	
	virtual void SetPointColor(FLinearColor InPointColor) override
	{
		if (PointColor != InPointColor)
		{
			PointColor = InPointColor;
			SetVerticesDirty();
		}
	}

	virtual float GetPointSize() override
	{
		return PointSize;
	}
	
	virtual void SetPointSize(float InPointSize) override
	{
		InPointSize = FMath::Max(0.1f, InPointSize);
		if (PointSize != InPointSize)
		{
			PointSize = InPointSize;
			SetVerticesDirty();
		}
	}

	virtual EPointMode GetPointMode() override
	{
		return PointMode;
	}

	virtual void SetPointMode(EPointMode InPointMode) override
	{
		if (PointMode != InPointMode)
		{
			PointMode = InPointMode;
			SetVerticesDirty();
		}
	}

};
