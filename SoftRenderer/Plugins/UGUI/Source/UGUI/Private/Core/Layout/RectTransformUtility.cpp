#include "Core/Layout/RectTransformUtility.h"
#include "UGUISubsystem.h"
#include "Core/MathUtility.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

bool FRectTransformUtility::RectangleIntersectRay(const URectTransformComponent* RectTransform, const FVector& WorldRayOrigin,
                                                  const FVector& WorldRayDir, bool bIgnoreReversedGraphics)
{
	if (!IsValid(RectTransform))
		return false;

	const auto WorldToLocal = RectTransform->GetComponentTransform().Inverse();
	const auto LocalSpaceRayOrigin = WorldToLocal.TransformPosition(WorldRayOrigin);
	auto LocalSpaceRayDir = WorldToLocal.TransformVector(WorldRayDir);
    LocalSpaceRayDir.Normalize();

	if (bIgnoreReversedGraphics)
	{
        if(FVector::DotProduct(LocalSpaceRayDir, FVector::UpVector) <= 0)
        {
            return false;
        }
	}
	
    const FRect& Rect = RectTransform->GetRect();

	FVector Corners[4];
    Corners[0] = FVector(Rect.XMin, Rect.YMin, 0.0f);
    Corners[1] = FVector(Rect.XMin, Rect.YMin + Rect.Height, 0.0f);
    Corners[2] = FVector(Rect.XMin + Rect.Width, Rect.YMin + Rect.Height, 0.0f);
    Corners[3] = FVector(Rect.XMin + Rect.Width, Rect.YMin, 0.0f);

    float T = 0;

    const VectorRegister VectorLocalRayOrigin = VectorLoadFloat3(&LocalSpaceRayOrigin);
    const VectorRegister VectorLocalRayDir = VectorLoadFloat3(&LocalSpaceRayDir);
    const VectorRegister VectorCorner0 = VectorLoadFloat3(&Corners[0]);
    const VectorRegister VectorCorner1 = VectorLoadFloat3(&Corners[1]);
    const VectorRegister VectorCorner2 = VectorLoadFloat3(&Corners[2]);
    
    bool bHit = false;
    bHit |= IntersectRayTriangle(VectorLocalRayOrigin, VectorLocalRayDir, VectorCorner0, VectorCorner1, VectorCorner2, T);
    if (!bHit)
    {
        const VectorRegister VectorCorner3 = VectorLoadFloat3(&Corners[3]);
        bHit |= IntersectRayTriangle(VectorLocalRayOrigin, VectorLocalRayDir, VectorCorner0, VectorCorner2, VectorCorner3, T);
    }
    
	return bHit;
}

DECLARE_CYCLE_STAT(TEXT("RectTransformUtility --- RectangleIntersectRayInWorldSpace"), STAT_UnrealGUI_RectangleIntersectRayInWorldSpace, STATGROUP_UnrealGUI);
bool FRectTransformUtility::RectangleIntersectRayInWorldSpace(const URectTransformComponent* RectTransform, const FVector& WorldRayOrigin, const FVector& WorldRayDir, bool bIgnoreReversedGraphics,
    const FVector& WorldCorner0, const FVector& WorldCorner1, const FVector& WorldCorner2, const FVector& WorldCorner3)
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_RectangleIntersectRayInWorldSpace);
    
    if (bIgnoreReversedGraphics)
    {
        const auto WorldToLocal = RectTransform->GetComponentTransform().Inverse();
        auto LocalSpaceRayDir = WorldToLocal.TransformVector(WorldRayDir);
        LocalSpaceRayDir.Normalize();
    	
        if(FVector::DotProduct(WorldRayDir, FVector::UpVector) <= 0)
        {
            return false;
        }
    }
    
    float T = 0;

    const VectorRegister VectorWorldRayOrigin = VectorLoadFloat3(&WorldRayOrigin);
    const VectorRegister VectorWorldRayDir = VectorLoadFloat3(&WorldRayDir);
    const VectorRegister VectorWorldCorner0 = VectorLoadFloat3(&WorldCorner0);
    const VectorRegister VectorWorldCorner1 = VectorLoadFloat3(&WorldCorner1);
    const VectorRegister VectorWorldCorner2 = VectorLoadFloat3(&WorldCorner2);
    
    bool bHit = false;
    bHit |= IntersectRayTriangle(VectorWorldRayOrigin, VectorWorldRayDir, VectorWorldCorner0, VectorWorldCorner1, VectorWorldCorner2, T);
    if (!bHit)
    {
        const VectorRegister VectorWorldCorner3 = VectorLoadFloat3(&WorldCorner3);
        bHit |= IntersectRayTriangle(VectorWorldRayOrigin, VectorWorldRayDir, VectorWorldCorner0, VectorWorldCorner2, VectorWorldCorner3, T);
    }
    
    return bHit;
}

DECLARE_CYCLE_STAT(TEXT("RectTransformUtility --- IntersectRayTriangle"), STAT_UnrealGUI_IntersectRayTriangle, STATGROUP_UnrealGUI);
bool FRectTransformUtility::IntersectRayTriangle(const VectorRegister& RayOrigin, const VectorRegister& RayDir, const VectorRegister& A,
                                                 const VectorRegister& B, const VectorRegister& C, float& OutT)
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_IntersectRayTriangle);
    
    /*constexpr float kMinDet = 1e-6f;

    // find vectors for two edges sharing vert0
    const FVector Edge1 = B - A;
    const FVector Edge2 = C - A;

    // begin calculating determinant - also used to calculate U parameter
    const FVector PVec = FVector::CrossProduct(RayDir, Edge2);

    // if determinant is near zero, ray lies in plane of triangle
    const float Det = FVector::DotProduct(Edge1, PVec);

    if (FMath::Abs(Det) < kMinDet)
        return false;

    const float InvDet = 1.0F / Det;

    // calculate distance from vert0 to ray origin
    const FVector TVec = RayOrigin - A;

    // calculate U parameter and test bounds
    const float U = FVector::DotProduct(TVec, PVec) * InvDet;
    if (U < 0.0 || U > 1.0)
        return false;

    // prepare to test V parameter
    const FVector QVec = FVector::CrossProduct(TVec, Edge1);

    // calculate V parameter and test bounds
    const float V = FVector::DotProduct(RayDir, QVec) * InvDet;
    if (V < 0.0f || U + V > 1.0f)
        return false;

    const float T = FVector::DotProduct(Edge2, QVec) * InvDet;
    if (T < 0.0f)
        return false;
	
    OutT = T;
    return true;*/
    
    constexpr float kMinDet = 1e-6f;
    
    /* find vectors for two edges sharing vert0 */
    const VectorRegister Edge1 = VectorSubtract(B, A);
    const VectorRegister Edge2 = VectorSubtract(C, A);

    /* begin calculating determinant - also used to calculate U parameter */
    const VectorRegister PVec = VectorCross(RayDir, Edge2);

    /* if determinant is near zero, ray lies in plane of triangle */
    const float Det = VectorGetComponent(VectorDot3(Edge1, PVec), 0);

    if (FMath::Abs(Det) < kMinDet)
        return false;

    const float InvDet = 1.0F / Det;

    /* calculate distance from vert0 to ray origin */
    const VectorRegister TVec = VectorSubtract(RayOrigin, A);

    /* calculate U parameter and test bounds */
    const float U = VectorGetComponent(VectorDot3(TVec, PVec), 0) * InvDet;
    if (U < 0.0 || U > 1.0)
        return false;

    /* prepare to test V parameter */
    const VectorRegister QVec = VectorCross(TVec, Edge1);

    /* calculate V parameter and test bounds */
    const float V = VectorGetComponent(VectorDot3(RayDir, QVec), 0) * InvDet;
    if (V < 0.0f || U + V > 1.0f)
        return false;

    const float T = VectorGetComponent(VectorDot3(Edge2, QVec), 0) * InvDet;
    if (T < 0.0f)
        return false;
	
    OutT = T;
    return true;
}

bool FRectTransformUtility::GetWorldRayFromCanvas(UCanvasSubComponent* Canvas, const FVector2D& ScreenPosition, FVector& WorldRayOrigin, FVector& WorldRayDirection)
{
    if (!IsValid(Canvas))
        return false;

    //Get screen position, convert to range 0-1
    const FVector2D ViewportSize = UUGUISubsystem::GetViewportSize(Canvas);
    FVector2D ViewPoint = ScreenPosition / ViewportSize;

    // If it's outside the client's viewport, do nothing
    /*if (ViewPoint.X < 0 || ViewPoint.X > 1 || ViewPoint.Y < 0 || ViewPoint.Y > 1)
        return false;*/

    const auto OverrideCanvas = Canvas->GetOrderOverrideCanvas();
    check(OverrideCanvas);
    
    if (OverrideCanvas->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay)
    {
        const auto RootCanvas = OverrideCanvas->GetRootCanvas();
        check(RootCanvas);
    
        if (!InternalGetWorldRayFromCanvas(ViewPoint, RootCanvas, WorldRayOrigin, WorldRayDirection))
        {
            return false;
        }
    }
    else
    {
        if (!InternalGetWorldRayFromViewport(OverrideCanvas, ScreenPosition, WorldRayOrigin, WorldRayDirection))
        {
            return false;
        }
    }

    return true;
}

bool FRectTransformUtility::ScreenPointToLocalPointInRectangle(const URectTransformComponent* RectTransform,
                                                               UCanvasSubComponent* Canvas, const FVector2D& ScreenPosition, FVector2D& LocalPosition)
{
    if (!IsValid(RectTransform))
        return false;

    FVector WorldRayOrigin;
    FVector WorldRayDir;
    if (!GetWorldRayFromCanvas(Canvas, ScreenPosition, WorldRayOrigin, WorldRayDir))
        return false;

    const auto WorldToLocal = RectTransform->GetComponentTransform().Inverse();
    const auto LocalSpaceRayOrigin = WorldToLocal.TransformPosition(WorldRayOrigin);
    auto LocalSpaceRayDir = WorldToLocal.TransformVector(WorldRayDir);
    LocalSpaceRayDir.Normalize();

    const FVector PlaneNormal = FVector(0, 0, 1);
    const FVector PlaneOrigin = FVector::ZeroVector;

    const float A = FVector::DotProduct(LocalSpaceRayDir, PlaneNormal);
    if (FMathUtility::Approximately(A, 0.0f))
    {
        return false;
    }

    const float Distance = FVector::DotProduct((PlaneOrigin - LocalSpaceRayOrigin), PlaneNormal) / A;
    LocalPosition = FVector2D(LocalSpaceRayOrigin + LocalSpaceRayDir * Distance);
    return true;	
}

bool FRectTransformUtility::ScreenPointToLocalPointInRectangle(const URectTransformComponent* RectTransform,
                                                               const FVector& WorldRayOrigin, const FVector& WorldRayDirection, FVector2D& LocalPosition)
{
    if (!IsValid(RectTransform))
        return false;
	
    const auto WorldToLocal = RectTransform->GetComponentTransform().Inverse();
    const auto LocalSpaceRayOrigin = WorldToLocal.TransformPosition(WorldRayOrigin);
    auto LocalSpaceRayDir = WorldToLocal.TransformVector(WorldRayDirection);
    LocalSpaceRayDir.Normalize();

    const FVector PlaneNormal = FVector(0, 0, 1);
    const FVector PlaneOrigin = FVector::ZeroVector;

    const float A = FVector::DotProduct(LocalSpaceRayDir, PlaneNormal);
    if (FMathUtility::Approximately(A, 0.0f))
    {
        return false;
    }

    const float Distance = FVector::DotProduct((PlaneOrigin - LocalSpaceRayOrigin), PlaneNormal) / A;
    LocalPosition = FVector2D(LocalSpaceRayOrigin + LocalSpaceRayDir * Distance);
    return true;
}

bool FRectTransformUtility::LocalPointToScreenPoint(URectTransformComponent* RectTransform, UCanvasSubComponent* Canvas,
	const FVector& LocalPosition, FVector2D& ScreenPosition)
{
    ScreenPosition = FVector2D(-1, -1);
	
    if (!IsValid(RectTransform))
        return false;

    if (!IsValid(Canvas))
    {
        Canvas = RectTransform->GetOwnerCanvas();
    }

    if (!IsValid(Canvas))
        return false;

    const auto OverrideCanvas = Canvas->GetOrderOverrideCanvas();
    check(OverrideCanvas);

    FMatrix OutViewProjectionMatrix;
    
    if (OverrideCanvas->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay)
    {
        const auto RootCanvas = OverrideCanvas->GetRootCanvas();
        check(RootCanvas);
        
        if (!InternalGetViewProjectionMatrixFromCanvas(RootCanvas, OutViewProjectionMatrix))
        {
            return false;
        }
    }
    else
    {
        if (!InternalGetViewProjectionMatrixFromViewport(OverrideCanvas, OutViewProjectionMatrix))
        {
            return false;
        }
    }

    const FVector& WorldPos = RectTransform->GetComponentTransform().TransformPosition(LocalPosition);
    const FVector4& ClipSpacePos = OutViewProjectionMatrix.TransformPosition(WorldPos);
	
    if (ClipSpacePos.W > 0.0f)
    {
        // the result of this will be x and y coords in -1..1 projection space
        const float RHW = 1.0f / ClipSpacePos.W;

        // Move from projection space to normalized 0..1 UI space
        const float NormalizedX = (ClipSpacePos.X * RHW * 0.5f) + 0.5f;
        //const float NormalizedY = 1.f - (ClipSpacePos.Y * RHW * 0.5f) - 0.5f;
        const float NormalizedY = -(ClipSpacePos.Y * RHW * 0.5f) + 0.5f;
    	const float NormalizedZ = ClipSpacePos.Z * RHW;

    	ScreenPosition.X = NormalizedX;
    	ScreenPosition.Y = NormalizedY;

    	const FVector2D& ViewportSize = UUGUISubsystem::GetViewportSize(RectTransform);
    	ScreenPosition = ScreenPosition * ViewportSize;
        
    	if (NormalizedX >= 0 && NormalizedX <= 1 && NormalizedY >= 0 && NormalizedY <= 1 && NormalizedZ >= 0 && NormalizedZ <= 1)
    	{
    		return true;
    	}
    }

    return false;
}

bool FRectTransformUtility::ScreenPointToLocalPoint(const URectTransformComponent* RectTransform, UCanvasSubComponent* Canvas,
                                                    const FVector2D& ScreenPosition, FVector2D& LocalPosition)
{
	if (!IsValid(RectTransform))
		return false;

	if (!IsValid(Canvas))
	{
		Canvas = RectTransform->GetOwnerCanvas();
	}

	if (!IsValid(Canvas))
		return false;

	return ScreenPointToLocalPointInRectangle(RectTransform, Canvas, ScreenPosition, LocalPosition);
}

bool FRectTransformUtility::WorldPointToSceneScreenPoint(UObject* WorldContextObject, FVector2D& ScreenPosition, FVector2D& ScreenToCenterDir, const FVector& WorldPosition,
    const bool bCalculateAngle, float& Angle)
{
    FMatrix OutViewMatrix;
    FMatrix OutViewProjectionMatrix;
    
    if (!InternalGetViewProjectionMatrixFromViewport(WorldContextObject, OutViewMatrix, OutViewProjectionMatrix))
    {
        return false;
    }

    bool bIsInScreen = false;
    bool bIsOverNearClipPlane;
    
    const FVector4& ClipSpacePos = OutViewProjectionMatrix.TransformPosition(WorldPosition);
    if (ClipSpacePos.W != 0.0f)
    {
        // the result of this will be x and y coords in -1..1 projection space
        const float RHW = 1.0f / ClipSpacePos.W;

        const float NormalizedZ = ClipSpacePos.Z * RHW ;
        if (FMath::Abs(NormalizedZ) <= 1)
        {
            bIsOverNearClipPlane = true;
            
            // Move from projection space to normalized 0..1 UI space
            float NormalizedX = (ClipSpacePos.X * RHW * 0.5f) + 0.5f;
            //const float NormalizedY = 1.f - (ClipSpacePos.Y * RHW * 0.5f) - 0.5f;
            float NormalizedY = -(ClipSpacePos.Y * RHW * 0.5f) + 0.5f;
            
            if (ClipSpacePos.W < 0)
            {
                NormalizedX = 1 - NormalizedX;
                NormalizedY = 1 -NormalizedY;
            }

            ScreenPosition.X = NormalizedX;
            ScreenPosition.Y = NormalizedY;

            const FVector2D& ViewportSize = UUGUISubsystem::GetViewportSize(WorldContextObject);
            ScreenPosition = ScreenPosition * ViewportSize;
        
            if (NormalizedX >= 0 && NormalizedX <= 1 && NormalizedY >= 0 && NormalizedY <= 1
                && NormalizedZ >= 0 && NormalizedZ <= 1)
            {
                bIsInScreen = true;
            }

            ScreenToCenterDir = (ScreenPosition - ViewportSize * 0.5f).GetSafeNormal();
            ScreenToCenterDir.Y = -ScreenToCenterDir.Y;
        }
        else
        {
            bIsOverNearClipPlane = false;
        }
    }
    else
    {
        bIsOverNearClipPlane = false;
    }

    if (!bIsOverNearClipPlane)
    {
        const FVector4& ViewSpacePos = OutViewProjectionMatrix.TransformPosition(WorldPosition);
        ScreenToCenterDir = FVector2D(ViewSpacePos.X, ViewSpacePos.Y).GetSafeNormal();
    }

    if (ScreenToCenterDir.IsZero())
    {
        ScreenToCenterDir = FVector2D(-1, 0);
    }

    if (bCalculateAngle)
    {
        const static FVector2D UpDir = FVector2D(0, 1);
        const static FVector2D DownDir = FVector2D(0, -1);
        const auto DirAngle = FMath::Acos(FMath::Clamp(FVector2D::DotProduct(UpDir, ScreenToCenterDir), -1.f, 1.f)) * 57.29578f;
        const auto CrossProduct = FVector2D::CrossProduct(ScreenToCenterDir, DownDir);
        Angle =  CrossProduct >= 0 ? DirAngle : -DirAngle;
    }
    
    return bIsInScreen;
}

bool FRectTransformUtility::ScreenPositionToParentBorder(const URectTransformComponent* RectTransform, const FVector2D& Dir, FVector2D& BorderPoint)
{
	if (!IsValid(RectTransform))
    {
        return false;
    }

    const auto ParentRectTransform = Cast<URectTransformComponent>(RectTransform->GetAttachParent());
    if (!IsValid(ParentRectTransform))
    {
        return false;
    }

    if (Dir.IsZero())
    {
        return false;
    }

    FVector2D BottomLeft = FVector2D(1e6, 1e6);
    FVector2D TopRight = FVector2D(-1e6, -1e6);
    
    FVector Corners[4];
    RectTransform->GetLocalCorners(Corners);
    const auto& LocalToParent = RectTransform->GetRelativeTransform();
    for (int32 Index = 0; Index < 4; ++Index)
    {
        const FVector Point = LocalToParent.TransformPosition(Corners[Index]);
        if (Point.X < BottomLeft.X)
        {
            BottomLeft.X = Point.X;
        }
        
        if (Point.X > TopRight.X)
        {
            TopRight.X = Point.X;
        }

        if (Point.Y < BottomLeft.Y)
        {
            BottomLeft.Y = Point.Y;
        }

        if (Point.Y > TopRight.Y)
        {
            TopRight.Y = Point.Y;
        }
    }
    
    const auto ParentRect = ParentRectTransform->GetRect();
    const auto ParentSize = (ParentRect.GetSize() - (TopRight - BottomLeft))* 0.5;
    
    BorderPoint = FVector2D::ZeroVector;
    if (!FMath::IsNearlyZero(Dir.X))
    {
        const float WidthScale = FMath::Abs(ParentSize.X / Dir.X);
        BorderPoint = FVector2D(WidthScale * Dir.X, WidthScale * Dir.Y);
        if (FMath::Abs(BorderPoint.Y) > ParentSize.Y)
        {
            if (!FMath::IsNearlyZero(Dir.Y))
            {
                const float HeightScale = FMath::Abs(ParentSize.Y / Dir.Y);
                BorderPoint = FVector2D(HeightScale * Dir.X, HeightScale * Dir.Y);
            	return true;
            }
            else
            {
                return false;
            }
        }
    }
    else if (!FMath::IsNearlyZero(Dir.Y))
    {
        const float HeightScale = FMath::Abs(ParentSize.Y / Dir.Y);
        BorderPoint = FVector2D(HeightScale * Dir.X, HeightScale * Dir.Y);
    	return true;
    }
    else
    {
        return false;
    }
	return false;
}


bool FRectTransformUtility::RectangleContainsScreenPoint(const URectTransformComponent* RectTransform,
                                                         UCanvasSubComponent* Canvas, const FVector2D& ScreenPosition, FVector& LocalPosition, bool bAlwaysGetLocalPosition)
{
    if (!IsValid(RectTransform))
        return false;

    FVector WorldRayOrigin;
    FVector WorldRayDir;
    if (!GetWorldRayFromCanvas(Canvas, ScreenPosition, WorldRayOrigin, WorldRayDir))
        return false;
    
    if (RectangleIntersectRay(RectTransform, WorldRayOrigin, WorldRayDir, false))
    {
        return true;
    }

    if (bAlwaysGetLocalPosition)
    {
        FVector2D LocalPosition2D;
        if (ScreenPointToLocalPointInRectangle(RectTransform, WorldRayOrigin, WorldRayDir, LocalPosition2D))
        {
            LocalPosition.X = LocalPosition2D.X;
            LocalPosition.Y = LocalPosition2D.Y;
            LocalPosition.Z = 0;
        }
    }
	
    return false;
}

void FRectTransformUtility::FlipLayoutOnAxis(URectTransformComponent* RectTransform, int32 Axis, bool bKeepPositioning, bool bRecursive)
{
    if (!IsValid(RectTransform))
        return;

    if (bRecursive)
    {
        const auto& BehaviourChildren = RectTransform->GetAttachChildren();
        for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
        {
            const auto ChildRect = Cast<URectTransformComponent>(BehaviourChildren[Index]);
            if (IsValid(ChildRect))
            {
                FlipLayoutOnAxis(ChildRect, Axis, false, true);
            }
        }
    }

    FVector2D Pivot = RectTransform->GetPivot();
    Pivot[Axis] = 1.0f - Pivot[Axis];
    RectTransform->SetPivot(Pivot);
    
    if (bKeepPositioning)
        return;

    FVector2D AnchoredPosition = RectTransform->GetAnchoredPosition();
    AnchoredPosition[Axis] = -AnchoredPosition[Axis];

    FVector2D AnchorMin = RectTransform->GetAnchorMin();
    FVector2D AnchorMax = RectTransform->GetAnchorMax();
    
    const float Temp = AnchorMin[Axis];
    AnchorMin[Axis] = 1 - AnchorMax[Axis];
    AnchorMax[Axis] = 1 - Temp;
    
    RectTransform->SetAnchorAndPosition(AnchorMin, AnchorMax, AnchoredPosition);
}

bool FRectTransformUtility::InternalGetWorldRayFromViewport(const UObject* WorldContextObject, const FVector2D& ScreenPosition,
    FVector& OutRayOrigin, FVector& OutRayDirection)
{
    if (!IsValid(WorldContextObject))
        return false;
    
    const auto World = WorldContextObject->GetWorld();
    if (!IsValid(World))
        return false;

    const auto PlayerController = World->GetFirstPlayerController();
    if (!IsValid(PlayerController))
        return false;

    const auto LocalPlayer = PlayerController->GetLocalPlayer();
    if (IsValid(LocalPlayer) && IsValid(LocalPlayer->ViewportClient))
    {
        // get the projection data
        FSceneViewProjectionData ProjectionData;
        if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, EStereoscopicPass::eSSP_FULL, ProjectionData))
        {
            const auto ViewProMatrix = ProjectionData.ViewRotationMatrix * ProjectionData.ProjectionMatrix; //VieProjectionMatrix without position
            FMatrix const InvViewProjMatrix = ViewProMatrix.InverseFast();
            FSceneView::DeprojectScreenToWorld(ScreenPosition, ProjectionData.GetConstrainedViewRect(), InvViewProjMatrix, /*out*/ OutRayOrigin, /*out*/ OutRayDirection);

            //take position out from ViewProjectionMatrix, after deproject calculation, add position to result, this can avoid float precision issue. otherwise result ray will have some obvious bias
            OutRayOrigin += ProjectionData.ViewOrigin;
            return true;
        }
    }

    return false;
}

bool FRectTransformUtility::InternalGetWorldRayFromCanvas(FVector2D& ViewPoint, UCanvasSubComponent* CanvasComp,
	FVector& OutRayOrigin, FVector& OutRayDirection)
{
    if (!IsValid(CanvasComp))
        return false;

    FVector ViewLocation;
    FMatrix ViewRotationMatrix;
    FMatrix ProjectionMatrix;
    FMatrix ViewProjectionMatrix;
    FMatrix LocalToWorldMatrix;
    if (!CanvasComp->CalculateCanvasMatrices(ViewLocation, ViewRotationMatrix, ProjectionMatrix, ViewProjectionMatrix, LocalToWorldMatrix))
    {
        return false;
    }
    
    ViewPoint.Y = 1.0f - ViewPoint.Y;

    const FMatrix& InvViewProjMatrix = ViewProjectionMatrix.InverseFast();

    const float ScreenSpaceX = (ViewPoint.X - 0.5f) * 2.0f;
    const float ScreenSpaceY = (ViewPoint.Y - 0.5f) * 2.0f;

    // The start of the raytrace is defined to be at MouseX, MouseY, 1 in projection space (z=1 is near, z=0 is far - this gives us better precision)
    // To get the direction of the raytrace we need to use any z between the near and the far plane, so let's use (MouseX, MouseY, 0.5)
    const FVector4 RayStartProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
    const FVector4 RayEndProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 0.5f, 1.0f);

    // Projection (changing the W coordinate) is not handled by the FMatrix transforms that work with vectors, so multiplications
    // by the projection matrix should use homogeneous coordinates (i.e. FPlane).
    const FVector4 HGRayStartWorldSpace = InvViewProjMatrix.TransformFVector4(RayStartProjectionSpace);
    const FVector4 HGRayEndWorldSpace = InvViewProjMatrix.TransformFVector4(RayEndProjectionSpace);

    FVector RayStartWorldSpace(HGRayStartWorldSpace.X, HGRayStartWorldSpace.Y, HGRayStartWorldSpace.Z);
    FVector RayEndWorldSpace(HGRayEndWorldSpace.X, HGRayEndWorldSpace.Y, HGRayEndWorldSpace.Z);

    // divide vectors by W to undo any projection and get the 3-space coordinate 
    if (HGRayStartWorldSpace.W != 0.0f)
    {
        RayStartWorldSpace /= HGRayStartWorldSpace.W;
    }

    if (HGRayEndWorldSpace.W != 0.0f)
    {
        RayEndWorldSpace /= HGRayEndWorldSpace.W;
    }

    // Finally, store the results in the outputs
    OutRayOrigin = RayStartWorldSpace;
    OutRayDirection = (RayEndWorldSpace - RayStartWorldSpace).GetSafeNormal();

    return true;
}

bool FRectTransformUtility::InternalGetViewProjectionMatrixFromViewport(const UObject* WorldContextObject, FMatrix& OutViewProjectionMatrix)
{
    if (!IsValid(WorldContextObject))
        return false;
	
    const auto World = WorldContextObject->GetWorld();
    if (!IsValid(World))
        return false;

    const auto PlayerController = World->GetFirstPlayerController();
    if (!IsValid(PlayerController))
        return false;

    const auto LocalPlayer = PlayerController->GetLocalPlayer();
    if (IsValid(LocalPlayer) && IsValid(LocalPlayer->ViewportClient))
    {
        // get the projection data
        FSceneViewProjectionData ProjectionData;
        if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, EStereoscopicPass::eSSP_FULL, ProjectionData))
        {
            OutViewProjectionMatrix = FTranslationMatrix(-ProjectionData.ViewOrigin) * ProjectionData.ViewRotationMatrix * ProjectionData.ProjectionMatrix;;
            return true;
        }
    }

    return false;
}

bool FRectTransformUtility::InternalGetViewProjectionMatrixFromViewport(const UObject* WorldContextObject, FMatrix& OutViewMatrix, FMatrix& OutViewProjectionMatrix)
{
    if (!IsValid(WorldContextObject))
        return false;
	
    const auto World = WorldContextObject->GetWorld();
    if (!IsValid(World))
        return false;

    const auto PlayerController = World->GetFirstPlayerController();
    if (!IsValid(PlayerController))
        return false;

    const auto LocalPlayer = PlayerController->GetLocalPlayer();
    if (IsValid(LocalPlayer) && IsValid(LocalPlayer->ViewportClient))
    {
        // get the projection data
        FSceneViewProjectionData ProjectionData;
        if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, EStereoscopicPass::eSSP_FULL, ProjectionData))
        {
            OutViewMatrix = FTranslationMatrix(-ProjectionData.ViewOrigin) * ProjectionData.ViewRotationMatrix;
            OutViewProjectionMatrix = OutViewMatrix * ProjectionData.ProjectionMatrix;;
            return true;
        }
    }

    return false;
}

bool FRectTransformUtility::InternalGetViewProjectionMatrixFromCanvas(UCanvasSubComponent* CanvasComp, FMatrix& OutViewProjectionMatrix)
{
    if (!IsValid(CanvasComp))
        return false;

    FVector ViewLocation;
    FMatrix ViewRotationMatrix;
    FMatrix ProjectionMatrix;
    FMatrix LocalToWorldMatrix;
    return CanvasComp->CalculateCanvasMatrices(ViewLocation, ViewRotationMatrix, ProjectionMatrix, OutViewProjectionMatrix, LocalToWorldMatrix);
}

FVector2D FRectTransformUtility::PixelAdjustPoint(const FVector2D& Point, const URectTransformComponent* RectTransform,
                                                  const UCanvasSubComponent* Canvas)
{
    if (!IsValid(Canvas) || Canvas->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_WorldSpace || Canvas->GetScaleFactor() == 0 || !Canvas->GetPixelPerfect())
        return Point;

    // Find the canvas responsible for pixel perfectness.
    const UCanvasSubComponent* CurrentCanvas = Canvas;
    const UCanvasSubComponent* RootCanvas = nullptr;
    while (IsValid(CurrentCanvas) && CurrentCanvas->GetPixelPerfect())
    {
        RootCanvas = CurrentCanvas;
        CurrentCanvas = CurrentCanvas->GetParentCanvas();
    }

    const URectTransformComponent* CanvasTransform = Cast<URectTransformComponent>(RootCanvas->GetComponent(URectTransformComponent::StaticClass()));
    if (!IsValid(CanvasTransform))
        return Point;

    const FTransform GraphicToCanvasMatrix = RectTransform->GetComponentTransform() * CanvasTransform->GetComponentTransform().Inverse();
    const FTransform CanvasToGraphicMatrix = GraphicToCanvasMatrix.Inverse();

    const FVector2D CanvasCorner = CanvasTransform->GetRect().GetPosition();

    const float Scale = Canvas->GetScaleFactor();
    const float ScaleInverse = 1.0f / Scale;

    // Convert to canvas space
    FVector Result = FVector(Point.X, Point.Y, 0.0f);
    Result = GraphicToCanvasMatrix.TransformPosition(Result);
    // Round to whole pixel in canvas pixel space
    Result = FVector(
        FMath::FloorToFloat((Result.X - CanvasCorner.X) * Scale + 0.5f) * ScaleInverse + CanvasCorner.X,
        FMath::FloorToFloat((Result.Y - CanvasCorner.Y) * Scale + 0.5f) * ScaleInverse + CanvasCorner.Y,
        Result.Z);
    // Convert back
    Result = CanvasToGraphicMatrix.TransformPosition(Result);

    return FVector2D(Result.X, Result.Y);
}

FRect FRectTransformUtility::PixelAdjustRect(const URectTransformComponent* RectTransform, const UCanvasSubComponent* Canvas)
{
    if (!IsValid(RectTransform))
        return FRect();

    FRect Rect = RectTransform->GetRect();
    if (!IsValid(Canvas) || Canvas->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_WorldSpace || Canvas->GetScaleFactor() == 0 || !Canvas->GetPixelPerfect())
        return Rect;

    // Find the canvas responsible for pixel perfectness.
    const UCanvasSubComponent* CurrentCanvas = Canvas;
    const UCanvasSubComponent* RootCanvas = nullptr;
    while (IsValid(CurrentCanvas) && CurrentCanvas->GetPixelPerfect())
    {
        RootCanvas = CurrentCanvas;
        CurrentCanvas = CurrentCanvas->GetParentCanvas();
    }

    const URectTransformComponent* CanvasTransform = Cast<URectTransformComponent>(RootCanvas->GetComponent(URectTransformComponent::StaticClass()));
    if (!IsValid(CanvasTransform))
        return Rect;

    const FTransform GraphicToCanvasMatrix = RectTransform->GetComponentTransform() * CanvasTransform->GetComponentTransform().Inverse();
    const FTransform CanvasToGraphicMatrix = GraphicToCanvasMatrix.Inverse();

    const FVector2D CanvasCorner =  CanvasTransform->GetRect().GetPosition();

    const float Scale = Canvas->GetScaleFactor();
    const float ScaleInverse = 1.0f / Scale;

    // Points for left, bottom, right, and top
    const FVector2D CenterPos = Rect.GetCenter();
    FVector Points[4];
    Points[0] = FVector(Rect.XMin, CenterPos.Y, 0.0f);
    Points[1] = FVector(CenterPos.X, Rect.YMin, 0.0f);
    Points[2] = FVector(Rect.GetXMax(), CenterPos.Y, 0.0f);
    Points[3] = FVector(CenterPos.X, Rect.GetYMax(), 0.0f);

	for (int32 Index = 0; Index < 4; ++Index)
	{
        // Convert to canvas space
        Points[Index] = GraphicToCanvasMatrix.TransformPosition(Points[Index]);
		
        // Round to whole numbers
        Points[Index] =
            FVector(
                FMath::FloorToFloat((Points[Index].X - CanvasCorner.X) * Scale + 0.5f) * ScaleInverse + CanvasCorner.X,
                FMath::FloorToFloat((Points[Index].Y - CanvasCorner.Y) * Scale + 0.5f) * ScaleInverse + CanvasCorner.Y,
                Points[Index].Z
            );
        // Convert back
        Points[Index] = CanvasToGraphicMatrix.TransformPosition(Points[Index]);
	}

    // Use rounded numbers
    Rect.XMin = Points[0].X;
    Rect.YMin = Points[1].Y;
    Rect.Width = Points[2].X - Points[0].X;
    Rect.Height = Points[3].Y - Points[1].Y;
	
    return Rect;
}

bool URectTransformLibrary::LocalPointToScreenPoint(URectTransformComponent* RectTransform,
	FVector LocalPosition, FVector2D& ScreenPosition, UCanvasSubComponent* Canvas)
{
    return FRectTransformUtility::LocalPointToScreenPoint(RectTransform, Canvas, LocalPosition, ScreenPosition);
}

bool URectTransformLibrary::ScreenPointToLocalPoint(URectTransformComponent* RectTransform, FVector2D ScreenPosition,
	FVector2D& LocalPosition, UCanvasSubComponent* Canvas)
{
	return FRectTransformUtility::ScreenPointToLocalPoint(RectTransform, Canvas, ScreenPosition, LocalPosition);
}

bool URectTransformLibrary::WorldPointToSceneScreenPoint(UObject* WorldContextObject, FVector2D& ScreenPosition,
    FVector2D& ScreenToCenterDir, const FVector& WorldPosition, const bool bCalculateAngle, float& Angle)
{
    return FRectTransformUtility::WorldPointToSceneScreenPoint(WorldContextObject, ScreenPosition, ScreenToCenterDir, WorldPosition, bCalculateAngle, Angle);
}

bool URectTransformLibrary::AlignToParentBorder(URectTransformComponent* RectTransform, const FVector2D& Dir)
{
    FVector2D BorderPoint;
	FRectTransformUtility::ScreenPositionToParentBorder(RectTransform, Dir, BorderPoint);
    RectTransform->SetLocalLocation(FVector(BorderPoint, 0));
    return true;
}