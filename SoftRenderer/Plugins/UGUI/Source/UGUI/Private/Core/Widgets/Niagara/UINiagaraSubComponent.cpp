#include "Core/Widgets/Niagara/UINiagaraSubComponent.h"
#include "Core/Widgets/Niagara/UINiagaraProxyComponent.h"

/////////////////////////////////////////////////////
// UUINiagaraSubComponent

UUINiagaraSubComponent::UUINiagaraSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NiagaraSystemAsset = nullptr;
	NiagaraComponent = nullptr;

	bActivateOnEnable = true;
	bDeactivateOnDisable = true;
	bActivateParticles = false;

	bRaycastTarget = false;
}

void UUINiagaraSubComponent::OnEnable()
{
	Super::OnEnable();

	SetNiagaraSystemAsset(NiagaraSystemAsset);

	if (bActivateOnEnable)
	{
		ActivateParticles();
	}
}

void UUINiagaraSubComponent::OnDisable()
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

void UUINiagaraSubComponent::UpdateGraphicEffects()
{
	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->SetGraphicEffects(GetUV1FromGraphicEffects(), true);
	}
}

void UUINiagaraSubComponent::UpdateRenderOpacity()
{
	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->SetRenderOpacity(GetRenderOpacityInHierarchy(), true);
	}
}

void UUINiagaraSubComponent::OnDestroy()
{
	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->DestroyComponent();
		NiagaraComponent = nullptr;
	}

	const auto CanvasRendererComp = GetCanvasRenderer();
	if (IsValid(CanvasRendererComp))
	{
		CanvasRendererComp->SetCustomRenderProxyComponent(nullptr);
	}
	
	Super::OnDestroy();
}

void UUINiagaraSubComponent::SetNiagaraSystemAsset(UNiagaraSystem* InNiagaraSystemAsset)
{
	if (NiagaraSystemAsset != InNiagaraSystemAsset)
		return;
	
	NiagaraSystemAsset = InNiagaraSystemAsset;

	if (IsValid(NiagaraSystemAsset))
	{
		if (!IsValid(NiagaraComponent) && IsValid(AttachTransform))
		{
			NiagaraComponent = NewObject<UUINiagaraProxyComponent>(AttachTransform->GetOwner(), NAME_None, RF_Transient);
#if WITH_EDITOR
			NiagaraComponent->bIsEditorOnly = true;
			NiagaraComponent->CreationMethod = EComponentCreationMethod::Instance;
#endif
			NiagaraComponent->bAllowAnyoneToDestroyMe = true;
			NiagaraComponent->bAutoActivate = false;
			NiagaraComponent->SetVisibility(false);
			NiagaraComponent->SetHiddenInGame(true);
			NiagaraComponent->SetupAttachment(AttachTransform);
			NiagaraComponent->SetGraphicEffects(GetUV1FromGraphicEffects(), false);
			NiagaraComponent->SetRenderOpacity(GetRenderOpacityInHierarchy(), false);
			NiagaraComponent->SetRelativeTransform(FTransform(FRotator(-90, 90, 0), FVector::ZeroVector, FVector::OneVector));
			NiagaraComponent->OnSystemFinished.AddUniqueDynamic(this, &UUINiagaraSubComponent::OnParticleSystemFinished);
			NiagaraComponent->RegisterComponent();

			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetCustomRenderProxyComponent(NiagaraComponent);
			}
		}
	}

	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->SetAsset(NiagaraSystemAsset);
		NiagaraComponent->MarkRenderStateDirty();

		if (IsValid(NiagaraComponent))
		{
			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetUseCustomRenderProxy(IsActiveAndEnabled());
			}

			if (IsActiveAndEnabled())
			{
				NiagaraComponent->SetHiddenInGame(false);
			}
		}
		else
		{
			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetUseCustomRenderProxy(false);
			}

			NiagaraComponent->SetHiddenInGame(true);
		}
	}
}

void UUINiagaraSubComponent::SetActivateParticles(bool bInActivateParticles)
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

void UUINiagaraSubComponent::ActivateParticles(bool bReset)
{
	if (IsValid(NiagaraComponent))
	{
		const auto CanvasRendererComp = GetCanvasRenderer();
		if (IsValid(CanvasRendererComp))
		{
			CanvasRendererComp->SetUseCustomRenderProxy(IsActiveAndEnabled());
		}

		if (IsActiveAndEnabled())
		{
			NiagaraComponent->SetHiddenInGame(false);

			if (NiagaraComponent->bCanPlayFx)
			{
				NiagaraComponent->Activate(bReset);
			}
			else
			{
				NiagaraComponent->bNeedPlayFX = true;
				NiagaraComponent->bReset = bReset;
			}
		}
	}
}

void UUINiagaraSubComponent::DeactivateParticles()
{
	if (IsValid(NiagaraComponent))
	{
		const auto CanvasRendererComp = GetCanvasRenderer();
		if (IsValid(CanvasRendererComp))
		{
			CanvasRendererComp->SetUseCustomRenderProxy(false);
		}

		NiagaraComponent->SetHiddenInGame(true);

		if (NiagaraComponent->bCanPlayFx)
		{
			NiagaraComponent->Deactivate();
		}
		else
		{
			NiagaraComponent->bNeedPlayFX = false;
		}
	}
}

void UUINiagaraSubComponent::OnParticleSystemFinished(UNiagaraComponent* FinishedComponent)
{
	if (IsValid(NiagaraComponent))
	{
		const auto CanvasRendererComp = GetCanvasRenderer();
		if (IsValid(CanvasRendererComp))
		{
			CanvasRendererComp->SetUseCustomRenderProxy(false);
		}

		NiagaraComponent->SetHiddenInGame(true);
	}
}

/////////////////////////////////////////////////////
