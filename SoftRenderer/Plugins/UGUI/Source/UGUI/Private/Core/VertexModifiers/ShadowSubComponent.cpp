#include "Core/VertexModifiers/ShadowSubComponent.h"
#include "UGUISettings.h"

/////////////////////////////////////////////////////
// UShadowSubComponent

constexpr float MaxEffectDistance = 600;

UShadowSubComponent::UShadowSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EffectColor = FLinearColor(0, 0, 0, 0.5f);
	EffectDistance = FVector(1, -1, 0);
	bUseGraphicAlpha = true;
	bUseExternalEffect = false;
	bFollowTransform = true;
	FixedLocalLocation = FVector(0, 0, 0);;
	ExternalEffectType = ECustomExternalEffectType::EffectType_None;
}

#if WITH_EDITORONLY_DATA

void UShadowSubComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (Graphic)
	{
		Graphic->SetVerticesDirty();
	}
}

#endif

void UShadowSubComponent::SetEffectColor(FLinearColor InEffectColor)
{
	if (EffectColor == InEffectColor)
		return;

	EffectColor = InEffectColor;

	if (Graphic)
	{
		Graphic->SetVerticesDirty();
	}
}

void UShadowSubComponent::SetUseExternalEffect(bool bInUseExternalEffect)
{
	if (bUseExternalEffect != bInUseExternalEffect)
	{
		bUseExternalEffect = bInUseExternalEffect;
		
		if (Graphic)
		{
			Graphic->SetVerticesDirty();
		}
	}
}

void UShadowSubComponent::SetEffectDistance(FVector InEffectDistance)
{
	if (InEffectDistance.X > MaxEffectDistance)
		InEffectDistance.X = MaxEffectDistance;
	if (InEffectDistance.X < -MaxEffectDistance)
		InEffectDistance.X = -MaxEffectDistance;

	if (InEffectDistance.Y > MaxEffectDistance)
		InEffectDistance.Y = MaxEffectDistance;
	if (InEffectDistance.Y < -MaxEffectDistance)
		InEffectDistance.Y = -MaxEffectDistance;

	if (EffectDistance == InEffectDistance)
		return;

	EffectDistance = InEffectDistance;

	if (Graphic)
	{
		Graphic->SetVerticesDirty();
	}
}

void UShadowSubComponent::SetUseGraphicAlpha(bool bInUseGraphicAlpha)
{
	if (bUseGraphicAlpha == bInUseGraphicAlpha)
		return;
	
	bUseGraphicAlpha = bInUseGraphicAlpha;

	if (Graphic)
	{
		Graphic->SetVerticesDirty();
	}
}

void UShadowSubComponent::SetExternalEffectType(TEnumAsByte<ECustomExternalEffectType> InExternalEffectType)
{
	if (ExternalEffectType == InExternalEffectType)
		return;
	
	ExternalEffectType = InExternalEffectType;

	if (Graphic)
	{
		Graphic->SetVerticesDirty();
	}
}

void UShadowSubComponent::SetFollowTransform(bool bInFollowTransform)
{
	if (bFollowTransform == bInFollowTransform)
		return;
	
	bFollowTransform = bInFollowTransform;

	if (Graphic)
	{
		Graphic->SetVerticesDirty();
	}
}

void UShadowSubComponent::SetFixedLocalLocation(FVector InFixedLocalLocation)
{
	if (FixedLocalLocation == InFixedLocalLocation)
		return;
	
	FixedLocalLocation = InFixedLocalLocation;

	if (Graphic)
	{
		Graphic->SetVerticesDirty();
	}
}

#if WITH_EDITORONLY_DATA

void UShadowSubComponent::OnCustomEffectChanged() const
{
	if (Graphic)
	{
		Graphic->SetVerticesDirty();
	}
}

#endif

void UShadowSubComponent::Awake()
{
	Super::Awake();

#if WITH_EDITORONLY_DATA
	UUGUISettings::Get()->OnCustomEffectChanged.AddUObject(this, &UShadowSubComponent::OnCustomEffectChanged);
#endif
}

void UShadowSubComponent::ModifyMesh(FVertexHelper& VertexHelper)
{
	if (!IsActiveAndEnabled())
		return;

	VertexHelper.GetUIVertexStream(FVertexHelper::VertexStream);

	FLinearColor FinalEffectColor = FLinearColor(0, 0, 0, 0.5f);
	FVector FinalEffectDistance = FVector(1, -1, 0);

	if (bUseExternalEffect)
	{
		const auto CustomEffectPtr = UUGUISettings::Get()->CustomEffectMap.Find(ExternalEffectType);
		if (CustomEffectPtr)
		{
			FinalEffectColor = CustomEffectPtr->EffectColor;
			FinalEffectDistance = CustomEffectPtr->EffectDistance;
		}
	}
	else
	{
		FinalEffectColor = EffectColor;
		FinalEffectDistance = EffectDistance;
	}

	if (!bFollowTransform)
	{
		const USceneComponent* OuterComp = Cast<USceneComponent>(GetOuter());
		if (IsValid(OuterComp))
		{
			FinalEffectDistance += FixedLocalLocation - OuterComp->GetRelativeLocation();
		}
	}

	ApplyShadow(FVertexHelper::VertexStream, FinalEffectColor, 0, FVertexHelper::VertexStream.Num(), FinalEffectDistance.X, FinalEffectDistance.Y, FinalEffectDistance.Z);

	VertexHelper.Reset();
	VertexHelper.AddUIVertexTriangleStream(FVertexHelper::VertexStream);
}

void UShadowSubComponent::OnTransformChanged()
{
	Super::OnTransformChanged();

	if (!bFollowTransform && Graphic)
	{
		Graphic->SetVerticesDirty();
	}
}

void UShadowSubComponent::ApplyShadow(TArray<FUIVertex>& Vertices, FLinearColor Color, int32 Start, int32 End, float X, float Y, float Z) const
{
	Vertices.Reserve(Vertices.Num() + End - Start);

	FLinearColor VertexColor = Color;
	for (int32 Index = Start; Index < End; ++Index)
	{
		FUIVertex Vt = Vertices[Index];
		Vertices.Add(Vt);
		
		Vt.Position.X += X;
		Vt.Position.Y += Y;
		Vt.Position.Z += Z;
		
		if (bUseGraphicAlpha)
		{
			VertexColor.A = Color.A * Vt.Color.A / 255.0f;
		}
		
		FastLinearColorToFColor(VertexColor, &Vt.Color);
		Vertices[Index] = Vt;
	}
}

/////////////////////////////////////////////////////
