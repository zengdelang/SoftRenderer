#include "Core/Layout/RectTransformComponent.h"
#include "Core/Layout/LayoutRebuilder.h"
#include "Core/Layout/RectTransformPreviewComponent.h"

/////////////////////////////////////////////////////
// URectTransformComponent

URectTransformComponent::URectTransformComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AnchorMin = FVector2D(0.5, 0.5);
	AnchorMax = FVector2D(0.5, 0.5);
	AnchoredPosition = FVector2D(0, 0);
	SizeDelta = FVector2D(100, 100);
	Pivot = FVector2D(0.5f, 0.5f);

#if WITH_EDITORONLY_DATA
	OffsetMin = GetOffsetMin();
	OffsetMax = GetOffsetMax();
	
	LocalPositionZ = 0;

	LocalRotation = FRotator::ZeroRotator;
	LocalScale = FVector::OneVector;
#endif
	
	bIsDetaching = false;
	
	USceneComponent::SetMobility(EComponentMobility::Movable);
	
	SetUsingAbsoluteLocation(false);
	SetUsingAbsoluteRotation(false);
	SetUsingAbsoluteScale(false);
	SetVisibility(false);

#if WITH_EDITOR
	bIsRootCanvas = false;
#endif
}

void URectTransformComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

#if WITH_EDITORONLY_DATA
	if (RectTransformPreview)
	{
		RectTransformPreview->DestroyComponent();
	}
#endif
}

void URectTransformComponent::AwakeFromLoad()
{
	if (!HasBeenAwaken())
	{
		UpdateRect();
	}

	Super::AwakeFromLoad();
	
#if WITH_EDITORONLY_DATA
	AActor* MyOwner = GetOwner();
	if ((MyOwner != nullptr) && !IsRunningCommandlet() && GetWorld() && !GetWorld()->IsGameWorld())
	{
		if (RectTransformPreview == nullptr)
		{
			RectTransformPreview = NewObject<URectTransformPreviewComponent>(MyOwner, NAME_None, RF_Transient);
			RectTransformPreview->SetupAttachment(this);
			RectTransformPreview->SetIsVisualizationComponent(true);
			RectTransformPreview->CreationMethod = CreationMethod;
			RectTransformPreview->RegisterComponentWithWorld(GetWorld());

			UpdateRect();
		}
	}
#endif
}

void URectTransformComponent::OnAttachmentChanged()
{
	if (!bIsDetaching)
	{
		InternalTransformParentChanged();		
	}
	
	Super::OnAttachmentChanged();
}

void URectTransformComponent::DetachFromComponent(const FDetachmentTransformRules& DetachmentRules)
{
	if (GetAttachParent() != nullptr)
	{
		bIsDetaching = true;
	}
	
	Super::DetachFromComponent(DetachmentRules);

	if (!bDisableDetachmentUpdateOverlaps && bIsDetaching)
	{
		InternalTransformParentChanged();
	}
	
	bIsDetaching = false;
}

#if WITH_EDITORONLY_DATA

void URectTransformComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{	
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(URectTransformComponent, AnchorMin))
	{
		SetAnchorMin(AnchorMin);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(URectTransformComponent, AnchorMax))
	{
		SetAnchorMax(AnchorMax);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(URectTransformComponent, AnchoredPosition))
	{
		SetAnchoredPosition(AnchoredPosition);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(URectTransformComponent, SizeDelta))
	{
		SetSizeDelta(SizeDelta);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(URectTransformComponent, Pivot))
	{
		SetPivot(Pivot);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(URectTransformComponent, OffsetMin))
	{
		SetOffsetMin(OffsetMin);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(URectTransformComponent, OffsetMax))
	{
		SetOffsetMax(OffsetMax);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(URectTransformComponent, LocalPositionZ))
	{
		SetLocalPositionZ(LocalPositionZ);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(URectTransformComponent, LocalRotation))
	{
		SetLocalRotation(LocalRotation);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(URectTransformComponent, LocalScale))
	{
		SetLocalScale(LocalScale);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void URectTransformComponent::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	URectTransformComponent* This = CastChecked<URectTransformComponent>(InThis);
	Collector.AddReferencedObject(This->RectTransformPreview);

	Super::AddReferencedObjects(InThis, Collector);
}

void URectTransformComponent::UpdateRectPreview() const
{
	if (RectTransformPreview)
	{
		RectTransformPreview->bIsRootCanvas = bIsRootCanvas;
		RectTransformPreview->AnchorMin = AnchorMin;
		RectTransformPreview->AnchorMax = AnchorMax;

		RectTransformPreview->SizeDelta = FVector2D(Rect.Width, Rect.Height);
		
		RectTransformPreview->XMin = Rect.XMin;
		RectTransformPreview->YMin = Rect.YMin;
		RectTransformPreview->XMax = Rect.XMin + Rect.Width;
		RectTransformPreview->YMax = Rect.YMin + Rect.Height;
		RectTransformPreview->MarkRenderStateDirty();
	}
}

#endif

#if WITH_EDITORONLY_DATA

void URectTransformComponent::UpdateRectTransformPreview()
{
	if (RectTransformPreview)
	{
		RectTransformPreview->bIsRootCanvas = bIsRootCanvas;
		
		RectTransformPreview->AnchorMin = AnchorMin;
		RectTransformPreview->AnchorMax = AnchorMax;
		
		RectTransformPreview->MarkRenderStateDirty();
	}
}

void URectTransformComponent::SetIsRootCanvas(bool bInIsRootCanvas)
{
	bIsRootCanvas = bInIsRootCanvas;
	if (RectTransformPreview)
	{
		RectTransformPreview->bIsRootCanvas = bIsRootCanvas;
		
		RectTransformPreview->AnchorMin = AnchorMin;
		RectTransformPreview->AnchorMax = AnchorMax;
		
		RectTransformPreview->MarkRenderStateDirty();
	}
}

#endif

void URectTransformComponent::InternalTransformParentChanged()
{
	Super::InternalTransformParentChanged();
	
	if (!IsRegistered() || bDisableTransformParentChanged)
		return;
	
	if (IsPendingKillOrUnreachable())
		return;

	if (GetWorld() == nullptr)
		return;

	if (IsBeingDestroyed())
		return;

	UpdateRect();
}

void URectTransformComponent::SetAnchorMin(FVector2D InAnchorMin)
{
#if !WITH_EDITOR
	if (AnchorMin == InAnchorMin)
		return;
#endif
	
	AnchorMin = InAnchorMin;

	UpdateRect();
}

void URectTransformComponent::SetAnchorMax(FVector2D InAnchorMax)
{
#if !WITH_EDITOR
	if (AnchorMax == InAnchorMax)
		return;
#endif
	AnchorMax = InAnchorMax;

	UpdateRect();
}

bool URectTransformComponent::SetAnchoredPosition3D(FVector InValue)
{
	bool bTransformChanged = false;
	if (GetRelativeLocation().Z != InValue.Z)
	{
		GetRelativeLocation_DirectMutable().Z = InValue.Z;
		bTransformChanged = true;
	}

	if (!SetAnchoredPosition(FVector2D(InValue.X, InValue.Y)) && bTransformChanged)
	{
		UpdateComponentToWorld();
		DispatchOnTransformChanged(this);
		return true;
	}

	return false;
}

bool URectTransformComponent::SetAnchoredPosition(FVector2D InAnchoredPosition)
{
#if !WITH_EDITOR
	if (AnchoredPosition == InAnchoredPosition)
		return false;
#endif
	
	AnchoredPosition = InAnchoredPosition;

#if WITH_EDITORONLY_DATA
	OffsetMin = GetOffsetMin();
	OffsetMax = GetOffsetMax();
#endif
	
	return UpdateRect();
}

void URectTransformComponent::SetSizeDelta(FVector2D InSizeDelta)
{
#if !WITH_EDITOR
	if (SizeDelta == InSizeDelta)
		return;
#endif
	
	SizeDelta = InSizeDelta;

#if WITH_EDITORONLY_DATA
	OffsetMin = GetOffsetMin();
	OffsetMax = GetOffsetMax();
#endif
	
	UpdateRect();
}

void URectTransformComponent::SetPivot(FVector2D InPivot)
{
#if !WITH_EDITOR
	if (Pivot == InPivot)
		return;
#endif
	
	Pivot = InPivot;

#if WITH_EDITORONLY_DATA
	OffsetMin = GetOffsetMin();
	OffsetMax = GetOffsetMax();
#endif
	
	UpdateRect();
}

void URectTransformComponent::SetOffsetMin(FVector2D InOffsetMin)
{
	const FVector2D Offset = InOffsetMin - (AnchoredPosition - SizeDelta * Pivot);
	if (Offset.IsNearlyZero())
		return;
	
	const FVector2D AnchoredPosOffset = Offset * (FVector2D::UnitVector - Pivot);

	SizeDelta -= Offset;
	AnchoredPosition += AnchoredPosOffset;

#if WITH_EDITORONLY_DATA
	OffsetMin = InOffsetMin;
#endif
	
	UpdateRect();
}

void URectTransformComponent::SetOffsetMax(FVector2D InOffsetMax)
{
	const FVector2D Offset = InOffsetMax - (AnchoredPosition + SizeDelta * (FVector2D::UnitVector - Pivot));
	if (Offset.IsNearlyZero())
		return;
	
	const FVector2D AnchoredPosOffset = Offset * Pivot;

	SizeDelta += Offset;
	AnchoredPosition += AnchoredPosOffset;

#if WITH_EDITORONLY_DATA
	OffsetMax = InOffsetMax;
#endif
	
	UpdateRect();
}

void URectTransformComponent::SetInsetAndSizeFromParentEdge(ERectTransformEdge Edge, float Inset, float Size)
{
	const int32 Axis = (Edge == ERectTransformEdge::RectTransformEdge_Top || Edge == ERectTransformEdge::RectTransformEdge_Bottom) ? 1 : 0;
	const bool bEnd = (Edge == ERectTransformEdge::RectTransformEdge_Top || Edge == ERectTransformEdge::RectTransformEdge_Right);

	// Set anchorMin and anchorMax to be anchored to the chosen edge.
	const float AnchorValue = bEnd ? 1 : 0;
	FVector2D Anchor = AnchorMin;
	Anchor[Axis] = AnchorValue;
	AnchorMin = Anchor;

	Anchor = AnchorMax;
	Anchor[Axis] = AnchorValue;
	AnchorMax = Anchor;

	// Set size. Since anchors are together, size and sizeDelta means the same in this case.
	FVector2D SizeD = SizeDelta;
	SizeD[Axis] = Size;
	SizeDelta = SizeD;

	// Set inset.
	FVector2D PositionCopy = AnchoredPosition;
	PositionCopy[Axis] = bEnd ? -Inset - Size * (1 - Pivot[Axis]) : Inset + Size * Pivot[Axis];
	AnchoredPosition = PositionCopy;

	SetAnchorAndSizeAndPosition(AnchorMin, AnchorMax, SizeDelta, AnchoredPosition);
}

void URectTransformComponent::SetSizeWithCurrentAnchors(ERectTransformAxis Axis, float Size)
{
	const int32 AxisValue = static_cast<int32>(Axis);
	FVector2D SizeD = GetSizeDelta();
	SizeD[AxisValue] = Size - GetParentSize()[AxisValue] * (GetAnchorMax()[AxisValue] - GetAnchorMin()[AxisValue]);
	SetSizeDelta(SizeD);
}

void URectTransformComponent::GetLocalCorners(FVector(&Corners)[4]) const
{
	const float X0 = Rect.XMin;
	const float Y0 = Rect.YMin;
	const float X1 = X0 + Rect.Width;
	const float Y1 = Y0 + Rect.Height;

	Corners[0] = FVector(X0, Y0, 0);
	Corners[1] = FVector(X0, Y1, 0);
	Corners[2] = FVector(X1, Y1, 0);
	Corners[3] = FVector(X1, Y0, 0);
}

void URectTransformComponent::GetWorldCorners(FVector(&Corners)[4]) const
{
	GetLocalCorners(Corners);

	const auto& LocalToWorld = GetComponentTransform();
	for (int32 Index = 0; Index < 4; ++Index)
	{
		Corners[Index] = LocalToWorld.TransformPosition(Corners[Index]);
	}
}

void URectTransformComponent::SetLocalPositionZ(float NewZ)
{
	auto CurRelativeLocation = GetRelativeLocation();
	if (!FMath::IsNearlyEqual(CurRelativeLocation.Z, NewZ))
	{
		CurRelativeLocation.Z = NewZ;
		GetRelativeLocation_DirectMutable() = CurRelativeLocation;
		UpdateComponentToWorld();

		DispatchOnTransformChanged(this);

#if WITH_EDITORONLY_DATA
		UpdateRectPreview();
#endif
	}
}

void URectTransformComponent::SetLocalRotation(FRotator NewRotation)
{
	auto CurRelativeRotation = GetRelativeRotation();
	if (CurRelativeRotation != NewRotation)
	{
		CurRelativeRotation = NewRotation;
		GetRelativeRotation_DirectMutable() = CurRelativeRotation;
		UpdateComponentToWorld();

		DispatchOnTransformChanged(this);

#if WITH_EDITORONLY_DATA
		UpdateRectPreview();
#endif
	}
}

void URectTransformComponent::SetLocalScale(FVector NewScale)
{
	auto CurRelativeScale = GetRelativeScale3D();
	if (CurRelativeScale != NewScale)
	{
		CurRelativeScale = NewScale;
		GetRelativeScale3D_DirectMutable() = CurRelativeScale;
		UpdateComponentToWorld();

		DispatchOnTransformChanged(this);

#if WITH_EDITORONLY_DATA
		UpdateRectPreview();
#endif
	}
}

void URectTransformComponent::SetLocalTransform(FTransform NewTransform)
{
	bool bTransformChanged = false;
	
	const auto NewRotation = NewTransform.GetRotation().Rotator();
	auto CurRelativeRotation = GetRelativeRotation();
	if (CurRelativeRotation != NewRotation)
	{
		bTransformChanged = true;
		CurRelativeRotation = NewRotation;
		GetRelativeRotation_DirectMutable() = CurRelativeRotation;
	}

	const auto NewScale = NewTransform.GetScale3D();
	auto CurRelativeScale = GetRelativeScale3D();
	if (CurRelativeScale != NewScale)
	{
		bTransformChanged = true;
		CurRelativeScale = NewScale;
		GetRelativeScale3D_DirectMutable() = CurRelativeScale;
	}

	if (!SetLocalLocation(NewTransform.GetLocation()) && bTransformChanged)
	{
		UpdateComponentToWorld();
		DispatchOnTransformChanged(this);
	}
}

void URectTransformComponent::SetEnabled(bool bNewEnabled)
{
	const bool bOldIsEnabled = IsEnabledInHierarchy();
	Super::SetEnabled(bNewEnabled);

	if (bOldIsEnabled != IsEnabledInHierarchy())
	{
		FLayoutRebuilder::MarkLayoutForRebuild(this);
	}
}

bool URectTransformComponent::UpdateRect(bool bUpdateRect, bool bTransformChanged)
{
	if (bUpdateRect)
	{
		const auto& ParentRect = GetParentRect();
		const auto AnchoredPosOffset = ParentRect.GetSize() * (AnchorMin + (AnchorMax - AnchorMin) * Pivot) + AnchoredPosition;

		auto CurRelativeLocation = GetRelativeLocation();
		CurRelativeLocation.X = ParentRect.XMin + AnchoredPosOffset.X;
		CurRelativeLocation.Y = ParentRect.YMin + AnchoredPosOffset.Y;
		
		if (!CurRelativeLocation.Equals(GetRelativeLocation()))
		{
			GetRelativeLocation_DirectMutable() = CurRelativeLocation;
			UpdateComponentToWorld();

			bTransformChanged = true;
		}
		
		const auto CurSize = ParentRect.GetSize() * (AnchorMax - AnchorMin) + SizeDelta;
		const FRect NewRect = FRect(-CurSize.X * Pivot.X, -CurSize.Y * Pivot.Y, CurSize.X, CurSize.Y);

		bUpdateRect = NewRect != Rect;
		Rect = NewRect;
	}

	if (bUpdateRect || bTransformChanged)
	{
		if (bTransformChanged && HasBeenEnabled())
		{
			OnTransformChanged();

			const auto& AllSubComponents = GetAllSubComponents();
			for (const auto& SubComponent : AllSubComponents)
			{
				if (IsValid(SubComponent))
				{
					SubComponent->OnTransformChanged();
				}
			}
		}

		// Note: Do not change the attachment state of the child during this call.
		bool bNeedLoop = true;
		while(bNeedLoop)
		{
			bNeedLoop = false;
			ResetAttachChildChanged();

			const auto& BehaviourChildren = GetAttachChildren();
			for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
			{
				const auto ChildRectTransform = Cast<URectTransformComponent>(BehaviourChildren[Index]);
				if (IsValid(ChildRectTransform))
				{
					ChildRectTransform->UpdateRect(bUpdateRect, bTransformChanged);
				}
				
				if (IsAttachChildChanged())
				{
					bNeedLoop = true;
					break;
				}
			}
		}

		if (bUpdateRect)
		{
			InternalOnRectTransformDimensionsChange();
		}
	}

#if WITH_EDITORONLY_DATA
	UpdateRectPreview();
#endif

	return bTransformChanged;
}

void URectTransformComponent::DispatchOnTransformChanged(UBehaviourComponent* BehaviourComp) const
{
	if (!IsValid(BehaviourComp))
		return;

	BehaviourComp->OnTransformChanged();

	const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
	for (const auto& SubComponent : AllSubComponents)
	{
		if (IsValid(SubComponent))
		{
			SubComponent->OnTransformChanged();
		}
	}

	bool bNeedLoop = true;
	while(bNeedLoop)
	{
		bNeedLoop = false;
		BehaviourComp->ResetAttachChildChanged();

		const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
		for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
		{
			DispatchOnTransformChanged(Cast<UBehaviourComponent>(BehaviourChildren[Index]));
				
			if (BehaviourComp->IsAttachChildChanged())
			{
				bNeedLoop = true;
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////
