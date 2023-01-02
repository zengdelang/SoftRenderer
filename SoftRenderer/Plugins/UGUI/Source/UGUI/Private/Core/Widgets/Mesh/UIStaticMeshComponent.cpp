#include "Core/Widgets/Mesh/UIStaticMeshComponent.h"
#include "Core/Widgets/Mesh/UIStaticMeshProxyComponent.h"

/////////////////////////////////////////////////////
// UUIStaticMeshComponent

UUIStaticMeshComponent::UUIStaticMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StaticMesh = nullptr;
	StaticMeshComponent = nullptr;
	bRaycastTarget = false;
}

#if WITH_EDITOR

void UUIStaticMeshComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property != nullptr)
	{
		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_STRING_CHECKED(UUIStaticMeshComponent, OverrideMaterials) ||
			PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_STRING_CHECKED(UUIStaticMeshComponent, StaticMesh))
		{
			CleanUpOverrideMaterials();
		}
	}
}

void UUIStaticMeshComponent::CleanUpOverrideMaterials()
{
	bool bUpdated = false;

	//We have to remove material override Ids that are bigger then the material list
	if (OverrideMaterials.Num() > GetNumOverrideMaterials())
	{
		//Remove the override material id that are superior to the static mesh materials number
		const int32 RemoveCount = OverrideMaterials.Num() - GetNumOverrideMaterials();
		OverrideMaterials.RemoveAt(GetNumOverrideMaterials(), RemoveCount);
		bUpdated = true;
	}

	if (bUpdated)
	{
		if (IsValid(StaticMeshComponent))
		{
			StaticMeshComponent->OverrideMaterials = OverrideMaterials;
			StaticMeshComponent->MarkRenderStateDirty();
		}
	}
}

#endif

void UUIStaticMeshComponent::OnEnable()
{
	Super::OnEnable();

	SetStaticMesh(StaticMesh);
}

void UUIStaticMeshComponent::OnDisable()
{
	const auto CanvasRendererComp = GetCanvasRenderer();
	if (IsValid(CanvasRendererComp))
	{
		CanvasRendererComp->SetUseCustomRenderProxy(false);
	}

	Super::OnDisable();
}

void UUIStaticMeshComponent::UpdateGraphicEffects()
{
	if (IsValid(StaticMeshComponent))
	{
		StaticMeshComponent->SetGraphicEffects(GetUV1FromGraphicEffects(), true);
	}
}

void UUIStaticMeshComponent::UpdateRenderOpacity()
{
	if (IsValid(StaticMeshComponent))
	{
		StaticMeshComponent->SetRenderOpacity(GetRenderOpacityInHierarchy(), true);
	}
}

void UUIStaticMeshComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (IsValid(StaticMeshComponent))
	{
		StaticMeshComponent->DestroyComponent();
		StaticMeshComponent = nullptr;
	}

	const auto CanvasRendererComp = GetCanvasRenderer();
	if (IsValid(CanvasRendererComp))
	{
		CanvasRendererComp->SetCustomRenderProxyComponent(nullptr);
	}
	
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UUIStaticMeshComponent::SetStaticMesh(UStaticMesh* InStaticMesh)
{
	StaticMesh = InStaticMesh;

	if (IsValid(StaticMesh))
	{
		if (!IsValid(StaticMeshComponent) && IsValid(GetOwner()))
		{
			StaticMeshComponent = NewObject<UUIStaticMeshProxyComponent>(GetOwner(), NAME_None, RF_Transient);
#if WITH_EDITOR
			StaticMeshComponent->bIsEditorOnly = true;
			StaticMeshComponent->CreationMethod = EComponentCreationMethod::Instance;
#endif
			StaticMeshComponent->bAutoActivate = false;
			StaticMeshComponent->SetVisibility(false);
			StaticMeshComponent->SetHiddenInGame(true);
			StaticMeshComponent->SetupAttachment(this);
			StaticMeshComponent->SetGraphicEffects(GetUV1FromGraphicEffects(), false);
			StaticMeshComponent->SetRenderOpacity(GetRenderOpacityInHierarchy(), false);
			StaticMeshComponent->SetRelativeTransform(FTransform(FRotator(-90, 90, 0), FVector::ZeroVector, FVector::OneVector));
			StaticMeshComponent->RegisterComponent();

			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetCustomRenderProxyComponent(StaticMeshComponent);
			}
		}
	}

	if (IsValid(StaticMeshComponent))
	{
		if (StaticMeshComponent->GetStaticMesh() != StaticMesh)
		{
			if (StaticMesh != nullptr && StaticMesh->RenderData != nullptr && StaticMesh->RenderData->IsInitialized())
			{
				StaticMeshComponent->SetStaticMesh(StaticMesh);
			}

			StaticMeshComponent->OverrideMaterials = OverrideMaterials;
			StaticMeshComponent->MarkRenderStateDirty();
		}
		
		if (IsValid(StaticMesh))
		{
			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetUseCustomRenderProxy(IsActiveAndEnabled());
			}

			if (IsActiveAndEnabled())
			{
				StaticMeshComponent->SetHiddenInGame(false);
			}
		}
		else
		{
			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetUseCustomRenderProxy(false);
			}

			StaticMeshComponent->SetHiddenInGame(true);
		}
	}
}

int32 UUIStaticMeshComponent::GetNumOverrideMaterials() const
{
	if (GetStaticMesh())
	{
		return GetStaticMesh()->StaticMaterials.Num();
	}

	return 0;
}

UMaterialInterface* UUIStaticMeshComponent::GetOverrideMaterial(int32 MaterialIndex) const
{
	// If we have a base materials array, use that
	if (OverrideMaterials.IsValidIndex(MaterialIndex) && OverrideMaterials[MaterialIndex])
	{
		return OverrideMaterials[MaterialIndex];
	}
	// Otherwise get from static mesh

	return GetStaticMesh() ? GetStaticMesh()->GetMaterial(MaterialIndex) : nullptr;
}

void UUIStaticMeshComponent::SetOverrideMaterial(int32 ElementIndex, UMaterialInterface* InMaterial)
{
	if (ElementIndex >= 0)
	{
		if (OverrideMaterials.IsValidIndex(ElementIndex) && (OverrideMaterials[ElementIndex] == InMaterial))
		{
			// Do nothing, the material is already set
		}
		else
		{
			// Grow the array if the new index is too large
			if (OverrideMaterials.Num() <= ElementIndex)
			{
				OverrideMaterials.AddZeroed(ElementIndex + 1 - OverrideMaterials.Num());
			}

			// Set the material and invalidate things
			OverrideMaterials[ElementIndex] = InMaterial;

			if (IsValid(StaticMeshComponent))
			{
				StaticMeshComponent->SetMaterial(ElementIndex, InMaterial);
			}
		}
	}
}

/////////////////////////////////////////////////////
