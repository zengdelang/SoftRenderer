#pragma once

#include "CoreMinimal.h"
#include "TweenFloatSubComponent.h"
#include "Core/Render/CanvasRendererSubComponent.h"
#include "TweenAlphaSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Tween Alpha", HideEnableCheckBox = true))
class UGUI_API UTweenAlphaSubComponent : public UTweenFloatSubComponent
{
	GENERATED_UCLASS_BODY()

public:

protected:
	//~ Begin ITweenInterface Interface
	virtual void InternalStartTween() override;
	//~ End ITweenInterface Interface

	virtual float GetCurrent() override
	{
		GetRenderer();
		if (IsValid(Renderer))
		{
			return Renderer->GetColor().A;
		}
		return 1.0f;
	}

private:
	void GetRenderer();
	void UpdateAlpha(float InAlpha);

private:
	UPROPERTY(Transient)
	UCanvasRendererSubComponent* Renderer;

};