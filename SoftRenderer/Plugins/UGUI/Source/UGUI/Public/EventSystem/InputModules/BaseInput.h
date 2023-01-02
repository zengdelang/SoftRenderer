#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UGUIInputInterface.h"
#include "BaseInput.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UGUI_API UBaseInput : public UObject, public IUGUIInputInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(Transient)
	TScriptInterface<IEventViewportClientInterface> ViewportClient;
	
protected:
	void CacheViewportClient();
	
public:
	//~ Begin IUGUIInputInterface Interface
	
	virtual bool GetMouseButtonDown(int32 Button) override;
	virtual bool GetMouseButtonUp(int32 Button) override;
	virtual bool GetMouseButton(int32 Button) override;
	
	virtual FVector2D MousePosition() override;
	virtual float MouseScrollDelta() override;
	
	virtual int32 TouchCount() override;
	virtual FViewportTouchState* GetTouch(int32 Index) override;
	
	virtual float GetAxis(const FName& AxisName) override;
	virtual bool GetButtonDown(const FName& ButtonName) override;
	
	virtual bool PopKeyCharacterEvent(FKeyCharacterEvent& OutKeyCharacterEvent) override;
	
	//~ End IUGUIInputInterface Interface

};
