#pragma once

#include "CoreMinimal.h"
#include "RectTransformComponent.h"
#include "Core/CanvasElementInterface.h"
#include "LayoutRebuilder.generated.h"

UCLASS()
class UGUI_API ULayoutRebuilder : public UObject, public ICanvasElementInterface
{
	GENERATED_UCLASS_BODY()

protected:
	TWeakObjectPtr<URectTransformComponent> ToRebuildTransform;
	
	uint32 CachedHashCode = 0;

public:
	float CacheTime = 0;
	
public:
	//~ Begin ICanvasElementInterface Interface
	virtual const USceneComponent* GetTransform() const override
	{
		if (ToRebuildTransform.IsValid())
		{
			return ToRebuildTransform.Get();
		}
		return nullptr;
	};
	
	virtual void Rebuild(ECanvasUpdate Executing) override;
	virtual void LayoutComplete() override;
	virtual void GraphicUpdateComplete() override {};
	virtual bool IsDestroyed() override { return !ToRebuildTransform.IsValid(); }
	virtual FString ToString() override { return GetFName().ToString(); };
	virtual uint32 GetHashCode() override { return CachedHashCode;};
	//~ End ICanvasElementInterface Interface

public:
	void Initialize(URectTransformComponent* InRect)
	{
		ToRebuildTransform = InRect;
		CachedHashCode = GetTypeHash(Cast<UObject>(InRect));
	}

	void Clear()
	{
		ToRebuildTransform = nullptr;
		CachedHashCode = 0;
		CacheTime = FApp::GetCurrentTime();
	}

public:
	void PerformLayoutCalculation(URectTransformComponent* Rect, bool bIsHorizontal) const;
	void PerformLayoutControl(URectTransformComponent* Rect, bool bIsHorizontal) const;

};

class UGUI_API FLayoutRebuilder
{
public:
	static void Shutdown();
	
public:
	/**
	 * Forces an immediate rebuild of the layout element and child layout elements affected by the calculations.
	 *
	 * @param  LayoutRoot  The layout element to perform the layout rebuild on.
	 *
	 * Normal use of the layout system should not use this method. Instead MarkLayoutForRebuild should be used instead, which triggers a delayed layout rebuild during the next layout pass. The delayed rebuild automatically handles objects in the entire layout hierarchy in the correct order, and prevents multiple recalculations for the same layout elements.
     * However, for special layout calculation needs, ::ref::ForceRebuildLayoutImmediate can be used to get the layout of a sub-tree resolved immediately. This can even be done from inside layout calculation methods such as ILayoutController.SetLayoutHorizontal orILayoutController.SetLayoutVertical. Usage should be restricted to cases where multiple layout passes are unavaoidable despite the extra cost in performance.
	 */
	static void ForceRebuildLayoutImmediate(URectTransformComponent* LayoutRoot);

	/**
	 * Mark the given RectTransform as needing it's layout to be recalculated during the next layout pass.
	 *
	 * @param  Rect  Rect to rebuild.
	 */
	static void MarkLayoutForRebuild(URectTransformComponent* Rect);
	
private:
	static bool ValidController(URectTransformComponent* LayoutRoot);
	static void MarkLayoutRootForRebuild(URectTransformComponent* Controller);

public:
	static ULayoutRebuilder* GetLayoutRebuilder();
	static void ReleaseLayoutRebuilder(ULayoutRebuilder* Rebuilder);
	
private:
	static TArray<ULayoutRebuilder*> Rebuilders;

};
