#pragma once

#include "CoreMinimal.h"
#include "MaskableGraphicComponent.h"
#include "BackgroundGlitchElementInterface.h"
#include "Core/Widgets/UIGlitchDefinitions.h"
#include "BackgroundGlitchComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (PostProcess), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UBackgroundGlitchComponent : public UMaskableGraphicComponent, public IBackgroundGlitchElementInterface
{
	GENERATED_UCLASS_BODY()

protected:
	/** Whether or not use glitch. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BackgroundGlitch)
	uint8 bUseGlitch : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1, EditCondition = "bUseGlitch"), Category = BackgroundGlitch)
	float Strength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseGlitch"), Category = BackgroundGlitch)
	EUIGlitchType Method;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1, ClampMax = 10, EditCondition = "Method!=EUIGlitchType::UIGlitchType_None"), Category = BackgroundGlitch)
	int32 DownSampleAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Method==EUIGlitchType::UIGlitchType_AnalogNoiseAndRGBSplit"), Category = BackgroundGlitch)
	FUIGlitchAnalogNoiseAndRGBSplitSet UIGlitchAnalogNoiseAndRGBSplitSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Method==EUIGlitchType::UIGlitchType_ImageBlock"), Category = BackgroundGlitch)
	FUIGlitchImageBlockSet UIGlitchImageBlockSet;

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override;

public:
	virtual bool IsUseGlitch() override
	{
		return bUseGlitch;
	}

	virtual void SetUseGlitch(bool bInUseGlitch) override
	{
		bUseGlitch = bInUseGlitch;
		SetEnabled(bUseGlitch);
	}
	
	virtual float GetStrength() override
	{
		return Strength;
	}
	
	virtual void SetStrength(float InStrength) override
	{
		Strength = InStrength;
	}

	virtual EUIGlitchType GetMethod() override
	{
		return Method;
	}

	virtual void SetMethod(EUIGlitchType InMethod) override
	{
		Method = InMethod;
	}

	virtual int32 GetDownSampleAmount() override
	{
		return DownSampleAmount;
	}

	virtual void SetDownSampleAmount(int32 InDownSampleAmount) override
	{
		DownSampleAmount = InDownSampleAmount;
	}

	UFUNCTION(BlueprintCallable, Category = BackgroundGlitch)
	virtual FUIGlitchAnalogNoiseAndRGBSplitSet& GetUIGlitchAnalogNoiseAndRGBSplitSet()
	{
		return UIGlitchAnalogNoiseAndRGBSplitSet;
	}

	UFUNCTION(BlueprintCallable, Category = BackgroundGlitch)
	virtual void SetUIGlitchAnalogNoiseAndRGBSplitSet(const FUIGlitchAnalogNoiseAndRGBSplitSet& InGlitchAnalogNoiseAndRGBSplitSet)
	{
		UIGlitchAnalogNoiseAndRGBSplitSet = InGlitchAnalogNoiseAndRGBSplitSet;
	}

	UFUNCTION(BlueprintCallable, Category = BackgroundGlitch)
	virtual FUIGlitchImageBlockSet& GetUIGlitchImageBlockSet()
	{
		return UIGlitchImageBlockSet;
	}

	UFUNCTION(BlueprintCallable, Category = BackgroundGlitch)
	virtual void SetUIGlitchImageBlockSet(const FUIGlitchImageBlockSet& InUIGlitchImageBlockSet)
	{
		UIGlitchImageBlockSet = InUIGlitchImageBlockSet;
	}
	
};
