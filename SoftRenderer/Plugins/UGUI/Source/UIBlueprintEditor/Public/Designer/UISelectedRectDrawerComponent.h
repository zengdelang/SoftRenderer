#pragma once

#include "CoreMinimal.h"
#include "UISelectedRectDrawerProxyComponent.h"
#include "Core/Widgets/MaskableGraphicComponent.h"
#include "UISelectedRectDrawerComponent.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UIBLUEPRINTEDITOR_API UUISelectedRectDrawerComponent : public UMaskableGraphicComponent
{
	GENERATED_UCLASS_BODY()

protected:
	FVector2D TopLeft;
	FVector2D BottomRight;

	float DotAlpha = 1;
	bool bShowLine = false;

public:
	void SetShowLine(bool bInShowLine)
	{
		if (bShowLine != bInShowLine)
		{
			bShowLine = bInShowLine;
			if (IsValid(SelectedRectDrawerProxyComponent))
			{
				SelectedRectDrawerProxyComponent->bShow = bShowLine;
				SelectedRectDrawerProxyComponent->MarkRenderStateDirty();
			}
		}
	}

	void SetRect(FVector2D InTopLeft, FVector2D InBottomRight, float InDotAlpha)
	{
		bShowLine = true;
		if (IsValid(SelectedRectDrawerProxyComponent))
		{
			SelectedRectDrawerProxyComponent->bShow = bShowLine;
			SelectedRectDrawerProxyComponent->MarkRenderStateDirty();
		}
		
		if (TopLeft != InTopLeft)
		{
			TopLeft = InTopLeft;
			if (IsValid(SelectedRectDrawerProxyComponent))
			{
				SelectedRectDrawerProxyComponent->TopLeft = TopLeft;
				SelectedRectDrawerProxyComponent->MarkRenderStateDirty();
			}
		}

		if (DotAlpha != InDotAlpha)
		{
			DotAlpha = InDotAlpha;
			if (IsValid(SelectedRectDrawerProxyComponent))
			{
				SelectedRectDrawerProxyComponent->DotAlpha = DotAlpha;
				SelectedRectDrawerProxyComponent->MarkRenderStateDirty();
			}
		}

		if (BottomRight != InBottomRight)
		{
			BottomRight = InBottomRight;
			if (IsValid(SelectedRectDrawerProxyComponent))
			{
				SelectedRectDrawerProxyComponent->BottomRight = BottomRight;
				SelectedRectDrawerProxyComponent->MarkRenderStateDirty();
			}
		}
	}

protected:
	UPROPERTY(Transient)
	UUISelectedRectDrawerProxyComponent* SelectedRectDrawerProxyComponent;

public:
	virtual void Awake() override;
	virtual void OnDisable() override;
    
protected:
	virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override {};

public:
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	
};
