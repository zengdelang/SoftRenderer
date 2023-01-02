#pragma once

#include "CoreMinimal.h"
#include "RectTransformComponent.h"
#include "RectTransformUtility.generated.h"

class UGUI_API FRectTransformUtility : public UBlueprintFunctionLibrary
{
public:
	static bool RectangleIntersectRay(const URectTransformComponent* RectTransform, const FVector& WorldRayOrigin, const FVector& WorldRayDir, bool bIgnoreReversedGraphics);

	static bool RectangleIntersectRayInWorldSpace(const URectTransformComponent* RectTransform, const FVector& WorldRayOrigin, const FVector& WorldRayDir, bool bIgnoreReversedGraphics,
		const FVector& WorldCorner0, const FVector& WorldCorner1, const FVector& WorldCorner2, const FVector& WorldCorner3);
	
public:
	static bool IntersectRayTriangle(const VectorRegister& RayOrigin, const VectorRegister& RayDir, const VectorRegister& A, const VectorRegister& B, const VectorRegister& C, float& OutT);

public:
	static bool GetWorldRayFromCanvas(UCanvasSubComponent* Canvas, const FVector2D& ScreenPosition, FVector& WorldRayOrigin, FVector& WorldRayDirection);

	static bool ScreenPointToLocalPointInRectangle(const URectTransformComponent* RectTransform, UCanvasSubComponent* Canvas, const FVector2D& ScreenPosition, FVector2D& LocalPosition);
	static bool ScreenPointToLocalPointInRectangle(const URectTransformComponent* RectTransform, const FVector& WorldRayOrigin, const FVector& WorldRayDirection, FVector2D& LocalPosition);

public:
	static bool LocalPointToScreenPoint(URectTransformComponent* RectTransform, UCanvasSubComponent* Canvas, const FVector& LocalPosition, FVector2D& ScreenPosition);
	static bool ScreenPointToLocalPoint(const URectTransformComponent* RectTransform, UCanvasSubComponent* Canvas, const FVector2D& ScreenPosition, FVector2D& LocalPosition);

	static bool WorldPointToSceneScreenPoint(UObject* WorldContextObject, FVector2D& ScreenPosition, FVector2D& ScreenToCenterDir, const FVector& WorldPosition, const bool bCalculateAngle, float& Angle);
	
	static bool ScreenPositionToParentBorder(const URectTransformComponent* RectTransform, const FVector2D& Dir, FVector2D& OutBorderPoint);
	
public:
	static bool RectangleContainsScreenPoint(const URectTransformComponent* RectTransform, UCanvasSubComponent* Canvas, const FVector2D& ScreenPosition, FVector& LocalPosition, bool bAlwaysGetLocalPosition = false);

	static void FlipLayoutOnAxis(URectTransformComponent* RectTransform, int32 Axis, bool bKeepPositioning, bool bRecursive);
	
protected:
	static bool InternalGetWorldRayFromViewport(const UObject* WorldContextObject, const FVector2D& ScreenPosition, FVector& OutRayOrigin, FVector& OutRayDirection);
	static bool InternalGetWorldRayFromCanvas(FVector2D& ViewPoint01, UCanvasSubComponent* CanvasComp, FVector& OutRayOrigin, FVector& OutRayDirection);

	static bool InternalGetViewProjectionMatrixFromViewport(const UObject* WorldContextObject, FMatrix& OutViewProjectionMatrix);
	static bool InternalGetViewProjectionMatrixFromViewport(const UObject* WorldContextObject, FMatrix& OutViewMatrix, FMatrix& OutViewProjectionMatrix);
	static bool InternalGetViewProjectionMatrixFromCanvas(UCanvasSubComponent* CanvasComp, FMatrix& OutViewProjectionMatrix);
	
public:
	/**
	 * Clamp a point to the nearest canvas pixel.
	 *
	 * @parama  Point  a coordinate in elementTransform coordinate system
	 * @parama  RectTransform  the reference coordinate system
     * @parama  Canvas  the canvas containing elementTransform
	 */
	static FVector2D PixelAdjustPoint(const FVector2D& Point, const URectTransformComponent* RectTransform, const UCanvasSubComponent* Canvas);
	
	/**
	 * Return a rectangle representing the rectTransform clamped to the nearest canvas pixel.
	 *
	 * @param  RectTransform  the rectTransform to clamp
	 * @param  Canvas  the canvas containing rectTransform
	 */
	static FRect PixelAdjustRect(const URectTransformComponent* RectTransform, const UCanvasSubComponent* Canvas);
	
};

UCLASS()
class UGUI_API URectTransformLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = RectTransformUtility)
	static bool LocalPointToScreenPoint(URectTransformComponent* RectTransform, FVector LocalPosition, FVector2D& ScreenPosition, UCanvasSubComponent* Canvas = nullptr);

	UFUNCTION(BlueprintCallable, Category = RectTransformUtility)
	static bool ScreenPointToLocalPoint(URectTransformComponent* RectTransform, FVector2D ScreenPosition, FVector2D& LocalPosition, UCanvasSubComponent* Canvas = nullptr);

	UFUNCTION(BlueprintCallable, Category = RectTransformUtility)
	static bool WorldPointToSceneScreenPoint(UObject* WorldContextObject, FVector2D& ScreenPosition, FVector2D& ScreenToCenterDir, const FVector& WorldPosition, const bool bCalculateAngle, float& Angle);

	UFUNCTION(BlueprintCallable, Category = RectTransformUtility)
	static bool AlignToParentBorder(URectTransformComponent* RectTransform, const FVector2D& Dir);
};
