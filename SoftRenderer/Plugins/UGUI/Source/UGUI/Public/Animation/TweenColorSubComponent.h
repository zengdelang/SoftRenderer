#pragma once

#include "CoreMinimal.h"
#include "TweenBaseSubComponent.h"
#include "Core/Render/CanvasRendererSubComponent.h"
#include "TweenColorSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Tween Color", HideEnableCheckBox = true))
class UGUI_API UTweenColorSubComponent : public UTweenBaseSubComponent
{
	GENERATED_UCLASS_BODY()

public:

protected:
	//~ Begin ITweenInterface Interface
	virtual void InitTweenRunner() override;
	virtual void InternalPlay() override;
	virtual void InternalToggle() override;
	virtual void InternalStartTween() override;
	//~ End ITweenInterface Interface

	virtual FLinearColor GetCurrent()
	{ 
		GetRenderer();
		if (IsValid(Renderer))
		{
			return Renderer->GetColor();
		}
		return FLinearColor::White;
	}

protected:
	UPROPERTY(EditAnywhere, Category = TweenColor)
	FLinearColor From = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = TweenColor)
	FLinearColor To = FLinearColor::White;

private:
	void GetRenderer();
	void UpdateColor(FLinearColor InColor);

private:
	UPROPERTY(Transient)
	UCanvasRendererSubComponent* Renderer;

};