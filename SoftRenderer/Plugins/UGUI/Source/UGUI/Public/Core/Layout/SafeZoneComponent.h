#pragma once

#include "CoreMinimal.h"
#include "VerticalLayoutGroupComponent.h"
#include "Core/Render/CanvasSubComponent.h"
#include "SafeZoneComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Layout), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API USafeZoneComponent : public UVerticalLayoutGroupComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = Layout)
	FUIMargin ExtraPadding;

	/** If this safe zone should pad for the left side of the screen's safe zone */
	UPROPERTY(EditAnywhere, Category = Layout)
	uint8 bPadLeft : 1;

	/** If this safe zone should pad for the right side of the screen's safe zone */
	UPROPERTY(EditAnywhere, Category = Layout)
	uint8 bPadRight : 1;

	/** If this safe zone should pad for the top side of the screen's safe zone */
	UPROPERTY(EditAnywhere, Category = Layout)
	uint8 bPadTop : 1;

	/** If this safe zone should pad for the bottom side of the screen's safe zone */
	UPROPERTY(EditAnywhere, Category = Layout)
	uint8 bPadBottom : 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Layout)
    FUIMargin SafeAreaScale;

    UPROPERTY(Transient)
    UCanvasSubComponent* Canvas;
	
	FDelegateHandle OnSafeFrameChangedHandle;
    FDelegateHandle OnScaleFactorChangedHandle;
    FDelegateHandle OnRenderModeChangedHandle;

public:
    static FMargin GlobalSafeMargin;
    static bool bUseGlobalSafeMargin;

public:
    static void UseGlobalSafeMargin(bool bUse);
    static void UpdateGlobalSafeMargin(FMargin InGlobalSafeMargin);

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual  void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
public:
    //~ Begin BehaviourComponent Interface
    virtual void OnEnable() override;
    virtual void OnDisable() override;
	virtual void OnDestroy() override;
    virtual void OnCanvasHierarchyChanged() override;
    virtual void OnTransformParentChanged() override;
    //~ End BehaviourComponent Interface.

protected:
    UCanvasSubComponent* GetRootCanvas() const;
    bool SetRootCanvas(UCanvasSubComponent* NewRootCanvas);
	
	void UpdateSafeMargin();

public:
    UFUNCTION(BlueprintCallable, Category = SafeZone)
    FUIMargin GetExtraPadding() const
    {
        return ExtraPadding;
    }

    UFUNCTION(BlueprintCallable, Category = SafeZone)
    void SetExtraPadding(FUIMargin InExtraPadding)
    {
        if (ExtraPadding != InExtraPadding)
        {
            ExtraPadding = InExtraPadding;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = SafeZone)
    bool IsPadLeft() const
    {
        return bPadLeft;
    }

    UFUNCTION(BlueprintCallable, Category = SafeZone)
    void SetPadLeft(bool bInPadLeft)
    {
	    if (bPadLeft != bInPadLeft)
	    {
            bPadLeft = bInPadLeft;
            UpdateSafeMargin();
	    }
    }

    UFUNCTION(BlueprintCallable, Category = SafeZone)
    bool IsPadRight() const
    {
        return bPadRight;
    }

    UFUNCTION(BlueprintCallable, Category = SafeZone)
    void SetPadRight(bool bInPadRight)
    {
        if (bPadRight != bInPadRight)
        {
            bPadRight = bInPadRight;
            UpdateSafeMargin();
        }
    }

    UFUNCTION(BlueprintCallable, Category = SafeZone)
    bool IsPadTop() const
    {
        return bPadTop;
    }

    UFUNCTION(BlueprintCallable, Category = SafeZone)
    void SetPadTop(bool bInPadTop)
    {
        if (bPadTop != bInPadTop)
        {
            bPadTop = bInPadTop;
            UpdateSafeMargin();
        }
    }

    UFUNCTION(BlueprintCallable, Category = SafeZone)
    bool IsPadBottom() const
    {
        return bPadBottom;
    }

    UFUNCTION(BlueprintCallable, Category = SafeZone)
    void SetPadBottom(bool bInPadBottom)
    {
        if (bPadBottom != bInPadBottom)
        {
            bPadBottom = bInPadBottom;
            UpdateSafeMargin();
        }
    }

    UFUNCTION(BlueprintCallable, Category = SafeZone)
    FUIMargin GetSafeAreaScale() const
    {
        return SafeAreaScale;
    }

    UFUNCTION(BlueprintCallable, Category = SafeZone)
    void SetSafeAreaScale(FUIMargin InSafeAreaScale)
    {
        if (SafeAreaScale != InSafeAreaScale)
        {
            SafeAreaScale = InSafeAreaScale;
            UpdateSafeMargin();
        }
    }
};
