#pragma once

#include "CoreMinimal.h"
#include "Core/UICommonDefinitions.h"
#include "UObject/Interface.h"
#include "BackgroundGlitchElementInterface.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API UBackgroundGlitchElementInterface : public UInterface
{
	GENERATED_BODY()
};

class UGUI_API IBackgroundGlitchElementInterface
{
	GENERATED_BODY()

protected:
	TSharedPtr<FUIGlitchGraphicData> GraphicData;

public:
	UFUNCTION(BlueprintCallable, Category = BackgroundGlitch)
	virtual bool IsUseGlitch() { return false; }
	
	UFUNCTION(BlueprintCallable, Category = BackgroundGlitch)
	virtual void SetUseGlitch(bool bUseGlitch) {}

	UFUNCTION(BlueprintCallable, Category = BackgroundGlitch)
	virtual float GetStrength() { return 0; }

	UFUNCTION(BlueprintCallable, Category = BackgroundGlitch)
	virtual void SetStrength(float Strength) {}

	UFUNCTION(BlueprintCallable, Category = BackgroundGlitch)
	virtual EUIGlitchType GetMethod() { return EUIGlitchType::UIGlitchType_None; }

	UFUNCTION(BlueprintCallable, Category = BackgroundGlitch)
	virtual void SetMethod(EUIGlitchType InMethod) {}

	UFUNCTION(BlueprintCallable, Category = BackgroundGlitch)
	virtual int32 GetDownSampleAmount() { return 0; }

	UFUNCTION(BlueprintCallable, Category = BackgroundGlitch)
	virtual void SetDownSampleAmount(int32 InDownSampleAmount) {}

};
