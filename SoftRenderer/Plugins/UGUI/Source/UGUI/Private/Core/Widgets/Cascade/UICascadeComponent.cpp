#include "Core/Widgets/Cascade/UICascadeComponent.h"
#include "Core/Widgets/Cascade/UICascadeProxyComponent.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleRequired.h"

/////////////////////////////////////////////////////
// UUICascadeComponent

UUICascadeComponent::UUICascadeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ParticleTemplate = nullptr;

	ParticleSystemComponent = nullptr;

	bActivateOnEnable = true;
	bDeactivateOnDisable = true;
	bActivateParticles = false;

	bRaycastTarget = false;
}

#if WITH_EDITOR

void UUICascadeComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property != nullptr)
	{
		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_STRING_CHECKED(UUICascadeComponent, OverrideMaterials) ||
			PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_STRING_CHECKED(UUICascadeComponent, ParticleTemplate))
		{
			CleanUpOverrideMaterials();
		}
	}
}

void UUICascadeComponent::CleanUpOverrideMaterials()
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
		if (IsValid(ParticleSystemComponent))
		{
			ParticleSystemComponent->EmitterMaterials = OverrideMaterials;
			ParticleSystemComponent->MarkRenderStateDirty();
		}
	}
}

#endif

void UUICascadeComponent::OnEnable()
{
	Super::OnEnable();

	SetParticleTemplate(ParticleTemplate);
	
	if (bActivateOnEnable)
	{
		ActivateParticles();
	}
}

void UUICascadeComponent::OnDisable()
{
	if (bDeactivateOnDisable)
	{
		DeactivateParticles();
	}

	const auto CanvasRendererComp = GetCanvasRenderer();
	if (IsValid(CanvasRendererComp))
	{
		CanvasRendererComp->SetUseCustomRenderProxy(false);
	}

	Super::OnDisable();
}

void UUICascadeComponent::UpdateGraphicEffects()
{
	if (IsValid(ParticleSystemComponent))
	{
		ParticleSystemComponent->SetGraphicEffects(GetUV1FromGraphicEffects(), true);
	}
}

void UUICascadeComponent::UpdateRenderOpacity()
{
	if (IsValid(ParticleSystemComponent))
	{
		ParticleSystemComponent->SetRenderOpacity(GetRenderOpacityInHierarchy(), true);
	}
}

void UUICascadeComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (IsValid(ParticleSystemComponent))
	{
		ParticleSystemComponent->DestroyComponent();
		ParticleSystemComponent = nullptr;
	}

	const auto CanvasRendererComp = GetCanvasRenderer();
	if (IsValid(CanvasRendererComp))
	{
		CanvasRendererComp->SetCustomRenderProxyComponent(nullptr);
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UUICascadeComponent::SetParticleTemplate(UParticleSystem* InParticleTemplate)
{
	ParticleTemplate = InParticleTemplate;

	if (IsValid(ParticleTemplate))
	{
		if (!IsValid(ParticleSystemComponent) && IsValid(GetOwner()))
		{
			ParticleSystemComponent = NewObject<UUICascadeProxyComponent>(GetOwner(), NAME_None, RF_Transient);
#if WITH_EDITOR
			ParticleSystemComponent->bIsEditorOnly = true;
			ParticleSystemComponent->CreationMethod = EComponentCreationMethod::Instance;
#endif
			ParticleSystemComponent->bAllowAnyoneToDestroyMe = true;
			ParticleSystemComponent->bAutoActivate = false;
			ParticleSystemComponent->SetVisibility(false);
			ParticleSystemComponent->SetHiddenInGame(true);
			ParticleSystemComponent->SetupAttachment(this);
			ParticleSystemComponent->SetGraphicEffects(GetUV1FromGraphicEffects(), false);
			ParticleSystemComponent->SetRenderOpacity(GetRenderOpacityInHierarchy(), false);
			ParticleSystemComponent->SetRelativeTransform(FTransform(FRotator(-90, 90, 0), FVector::ZeroVector, FVector::OneVector));
			ParticleSystemComponent->OnSystemFinished.AddUniqueDynamic(this, &UUICascadeComponent::OnParticleSystemFinished);
			ParticleSystemComponent->RegisterComponent();

			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetCustomRenderProxyComponent(ParticleSystemComponent);
			}
		}
	}

	if (IsValid(ParticleSystemComponent))
	{
		if (ParticleSystemComponent->Template != ParticleTemplate)
		{
			ParticleSystemComponent->SetTemplate(ParticleTemplate);
			ParticleSystemComponent->EmitterMaterials = OverrideMaterials;
			ParticleSystemComponent->MarkRenderStateDirty();
		}
		
		if (IsValid(ParticleTemplate))
		{
			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetUseCustomRenderProxy(IsActiveAndEnabled());
			}

			if (IsActiveAndEnabled())
			{
				ParticleSystemComponent->SetHiddenInGame(false);
			}
		}
		else
		{
			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetUseCustomRenderProxy(false);
			}

			ParticleSystemComponent->SetHiddenInGame(true);
		}
	}
}

void UUICascadeComponent::SetActivateParticles(bool bInActivateParticles)
{
	if (bActivateParticles == bInActivateParticles)
		return;
	
	bActivateParticles = bInActivateParticles;
	if (bActivateParticles)
	{
		ActivateParticles();
	}
	else
	{
		DeactivateParticles();
	}
}

void UUICascadeComponent::ActivateParticles(bool bReset)
{
	if (IsValid(ParticleSystemComponent))
	{
		const auto CanvasRendererComp = GetCanvasRenderer();
		if (IsValid(CanvasRendererComp))
		{
			CanvasRendererComp->SetUseCustomRenderProxy(IsActiveAndEnabled());
		}

		if (IsActiveAndEnabled())
		{
			ParticleSystemComponent->SetHiddenInGame(false);

			if (ParticleSystemComponent->bCanPlayFx)
			{
				ParticleSystemComponent->Activate(bReset);
			}
			else
			{
				ParticleSystemComponent->bNeedPlayFX = true;
				ParticleSystemComponent->bReset = bReset;
			}
		}
	}
}

void UUICascadeComponent::DeactivateParticles()
{
	if (IsValid(ParticleSystemComponent))
	{
		const auto CanvasRendererComp = GetCanvasRenderer();
		if (IsValid(CanvasRendererComp))
		{
			CanvasRendererComp->SetUseCustomRenderProxy(false);
		}

		ParticleSystemComponent->SetHiddenInGame(true);

		if (ParticleSystemComponent->bCanPlayFx)
		{
			ParticleSystemComponent->Deactivate();
		}
		else
		{
			ParticleSystemComponent->bNeedPlayFX = false;
		}
	}
}

int32 UUICascadeComponent::GetNumOverrideMaterials() const
{
	if (ParticleTemplate)
	{
		return ParticleTemplate->Emitters.Num();
	}
	return 0;
}

UMaterialInterface* UUICascadeComponent::GetOverrideMaterial(int32 MaterialIndex) const
{
	if (OverrideMaterials.IsValidIndex(MaterialIndex) && OverrideMaterials[MaterialIndex] != nullptr)
	{
		return OverrideMaterials[MaterialIndex];
	}
	
	if (ParticleTemplate && ParticleTemplate->Emitters.IsValidIndex(MaterialIndex))
	{
		UParticleEmitter* Emitter = ParticleTemplate->Emitters[MaterialIndex];
		if (Emitter && Emitter->LODLevels.Num() > 0)
		{
			const UParticleLODLevel* EmitterLODLevel = Emitter->LODLevels[0];
			if (EmitterLODLevel && EmitterLODLevel->RequiredModule)
			{
				return EmitterLODLevel->RequiredModule->Material;
			}
		}
	}
	
	return nullptr;
}

void UUICascadeComponent::SetOverrideMaterial(int32 ElementIndex, UMaterialInterface* InMaterial)
{
	if (ParticleTemplate && ParticleTemplate->Emitters.IsValidIndex(ElementIndex))
	{
		if (!OverrideMaterials.IsValidIndex(ElementIndex))
		{
			OverrideMaterials.AddZeroed(ElementIndex + 1 - OverrideMaterials.Num());
		}
		OverrideMaterials[ElementIndex] = InMaterial;
	}

	if (IsValid(ParticleSystemComponent))
	{
		ParticleSystemComponent->SetMaterial(ElementIndex, InMaterial);
	}
}

void UUICascadeComponent::OnParticleSystemFinished(UParticleSystemComponent* FinishedComponent)
{
	if (IsValid(ParticleSystemComponent))
	{
		const auto CanvasRendererComp = GetCanvasRenderer();
		if (IsValid(CanvasRendererComp))
		{
			CanvasRendererComp->SetUseCustomRenderProxy(false);
		}

		ParticleSystemComponent->SetHiddenInGame(true);
	}
}

/////////////////////////////////////////////////////
