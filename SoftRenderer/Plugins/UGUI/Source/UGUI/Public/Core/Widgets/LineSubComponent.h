#pragma once

#include "CoreMinimal.h"
#include "LineElementInterface.h"
#include "PaperSprite.h"
#include "Sprite2D.h"
#include "Core/Widgets/MaskableGraphicSubComponent.h"
#include "Interfaces/SpriteTextureAtlasListenerInterface.h"
#include "LineSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Line"))
class UGUI_API ULineSubComponent : public UMaskableGraphicSubComponent, public ILineElementInterface, public ISpriteTextureAtlasListenerInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = Graphic)
	uint8 bClosedLine : 1;
	
	UPROPERTY(EditAnywhere, Category = Graphic, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float LineThickness;

	UPROPERTY(EditAnywhere, Category = Graphic)
	FLinearColor PointColor;

	UPROPERTY(EditAnywhere, Category = Graphic, meta = (ClampMin = "0.1", UIMin = "0.1"))
	float PointSize;
	
	UPROPERTY(EditAnywhere, Category = Graphic)
	EPointMode PointMode;

	UPROPERTY(EditAnywhere, Category = Graphic)
	ELineType LineType;
	
	UPROPERTY(EditAnywhere, Category = Graphic)
	TArray<FVector2D> LinePoints;

	UPROPERTY(EditAnywhere, Category = Graphic, meta = (ClampMin = 0.1f))
	float DrawLineTolerance;
	
	UPROPERTY(EditAnywhere, Category = Graphic, meta=(DisplayThumbnail="true", AllowedClasses="Sprite2D,SlateTextureAtlasInterface"))
	UObject* Sprite;

protected:
	virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override;

public:
	//~ Begin BehaviourSubComponent Interface
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	//~ End BehaviourSubComponent Interface.

	//~ Begin ISpriteTextureAtlasListenerInterface Interface
	virtual void NotifySpriteTextureChanged(bool bTextureChanged, bool bUVChanged) override
	{
		if (bUVChanged)
		{
			SetVerticesDirty();
		}

		if (bTextureChanged)
		{
			SetMaterialDirty();
		}
	}
	//~ End ISpriteTextureAtlasListenerInterface Interface.
	
public:
	virtual UTexture* GetMainTexture() const override
	{
		if (const auto PaperSprite = Cast<UPaperSprite>(Sprite))
		{
			return PaperSprite->GetSlateAtlasData().AtlasTexture;
		}
        
		if (const auto Sprite2D = Cast<USprite2D>(Sprite))
		{
			return Sprite2D->GetSpriteTexture();
		}
		
		return nullptr;
	}
	
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
	
	virtual bool GetClosedLine() const override
	{
		return bClosedLine;
	}
	
	virtual void SetClosedLine(bool bInClosedLine) override
	{
		if (bClosedLine != bInClosedLine)
		{
			bClosedLine = bInClosedLine;
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
	
	virtual TArray<FVector2D>& GetLinePoints() override
	{
		return LinePoints;
	}
	
	virtual void RedrawLine() override
	{
		SetVerticesDirty();
	}
	
	virtual void AddPoint(FVector2D InPoint) override
	{
		LinePoints.Emplace(InPoint);
		SetVerticesDirty();
	}
	
	virtual bool RemovePoint(int32 InPointIndex) override
	{
		if (LinePoints.IsValidIndex(InPointIndex))
		{
			LinePoints.RemoveAt(InPointIndex, 1, false);
			SetVerticesDirty();
			return true;
		}
		return false;
	}
	
	virtual void ClearPoints() override
	{
		if (LinePoints.Num() > 0)
		{
			LinePoints.Empty();
			SetVerticesDirty();
		}
	}

	virtual ELineType GetLineType() override
	{
		return LineType;
	}

	virtual void SetLineType(ELineType InLineType) override
	{
		if (LineType != InLineType)
		{
			LineType = InLineType;
			SetVerticesDirty();
		}
	}

	virtual float GetDrawLineTolerance() override
	{
		return DrawLineTolerance;
	}
	
	virtual void SetDrawLineTolerance(float InDrawLineTolerance) override
	{
		if (DrawLineTolerance != InDrawLineTolerance)
		{
			DrawLineTolerance = InDrawLineTolerance;
			SetVerticesDirty();
		}
	}

	virtual UObject* GetSprite() const override
	{
		return Sprite;
	}
	
	virtual void SetSprite(UObject* InSprite) override;
	
protected:
	virtual FRect GetTransformRect() override
	{
		if (IsValid(AttachTransform))
		{
			return AttachTransform->GetRect();
		}
		return FRect();
	}
};
