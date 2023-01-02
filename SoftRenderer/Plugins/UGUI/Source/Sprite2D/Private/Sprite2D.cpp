#include "Sprite2D.h"
#include "SpriteMergeSubsystem.h"
#include "Interfaces/SpriteTextureAtlasListenerInterface.h"

/////////////////////////////////////////////////////
// USprite2D

USprite2D::USprite2D(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseDynamicAtlas = true;
	bWaitingTextureData = false;
	
	SourceDimension = FVector2D::ZeroVector;
	
	SourceTexture = nullptr;
	ReferenceCount = 0;
	PixelsPerUnrealUnit = 1;

	AtlasSourceUV = FVector2D::ZeroVector;
}

#if WITH_EDITOR

void USprite2D::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PixelsPerUnrealUnit <= 0.0f)
	{
		PixelsPerUnrealUnit = 1.0f;
	}
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USprite2D, SourceTexture))
	{
		if(OnSpriteChange.IsBound())
		{
			OnSpriteChange.Broadcast();
		}
		
		if (SourceTexture)
		{
			SourceDimension = FVector2D(SourceTexture->GetImportedSize());
		}
		else
		{
			SourceDimension = FVector2D::ZeroVector;
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

void USprite2D::FinishDestroy()
{
	check(IsInGameThread());
	
	if (SpriteAtlasTexture.IsValid())
	{	
		SpriteAtlasTexture.Pin()->IncrementInvalidSpriteCount();
		SpriteAtlasTexture.Reset();
	}
	
	UObject::FinishDestroy();
}

void USprite2D::AddSpriteListener(UObject* Listener)
{
	if (!IsValid(Listener))
		return;

	if (Listener->Implements<USpriteTextureAtlasListenerInterface>())
	{
		Listeners.Add(Listener);
	}
}

void USprite2D::RemoveSpriteListener(UObject* Listener)
{
	Listeners.Remove(Listener);
}

void USprite2D::IncreaseReferenceCount()
{
	++ReferenceCount;
	if (ReferenceCount == 1)
	{
		if (SpriteAtlasTexture.IsValid())
		{
			SpriteAtlasTexture.Pin()->IncrementValidSpriteCount();
		}
	}
}

void USprite2D::DecreaseReferenceCount()
{
	ReferenceCount = FMath::Max(0, ReferenceCount - 1);
	if (ReferenceCount == 0)
	{
		if (SpriteAtlasTexture.IsValid())
		{
			SpriteAtlasTexture.Pin()->DecrementValidSpriteCount();
		}
	}
}

UTexture2D* USprite2D::GetSpriteTexture()
{
	if (bUseDynamicAtlas)
	{
		if (!SpriteAtlasTexture.IsValid() && !DynamicSpriteTexture.IsValid() && GEngine && !bWaitingTextureData)
		{
			USpriteMergeSubsystem* Subsystem = GEngine->GetEngineSubsystem<USpriteMergeSubsystem>();
			if (IsValid(Subsystem))
			{
				DynamicSpriteTexture = Subsystem->GetSpriteTexture2D(this, AtlasName);
			}
		}
		
		return DynamicSpriteTexture.Get();
	}
	
	return SourceTexture;
}

UTexture2D* USprite2D::GetSpriteTextureRaw() const
{
	if (bUseDynamicAtlas)
	{
		return DynamicSpriteTexture.Get();
	}
	
	return SourceTexture;
}

#if WITH_EDITOR

bool USprite2D::GetIsMerged()
{
	if (DynamicSpriteTexture.IsValid())
	{
		return  true;
	}
	else
	{
		return  false;
	}
}


void USprite2D::RemergeSprite2D()
{
	if (bUseDynamicAtlas && SpriteAtlasTexture.IsValid())
	{
		SpriteAtlasTexture.Pin()->RemoveSprite(this);
		DynamicSpriteTexture.Reset();
		SpriteAtlasTexture.Reset();
		USpriteMergeSubsystem* Subsystem = GEngine->GetEngineSubsystem<USpriteMergeSubsystem>();
		if (IsValid(Subsystem))
		{
			DynamicSpriteTexture = Subsystem->GetSpriteTexture2D(this, AtlasName);
			if (DynamicSpriteTexture.IsValid())
			{
				NotifySpriteTextureChanged(true, true);
			}
		}
	}
}

#endif

FVector4 USprite2D::GetOuterUV()
{
	if (bUseDynamicAtlas)
	{
		if (!SpriteAtlasTexture.IsValid() && !bWaitingTextureData)
		{
			GetSpriteTexture();
		}

		if (DynamicSpriteTexture.IsValid())
		{
			const FVector2D ImportedSize = FVector2D(DynamicSpriteTexture->GetSizeX(), DynamicSpriteTexture->GetSizeY());
			// 0.5 to avoid bilinear bleeding
			const FVector2D TopLeftOuterUV = (AtlasSourceUV + FVector2D(0.5f, 0.5f)) / ImportedSize;
			return FVector4(TopLeftOuterUV,  (AtlasSourceUV + SourceDimension - FVector2D(0.5f, 0.5f)) / ImportedSize);
		}
	}
	else if (SourceTexture)
	{
		const FVector2D ImportedSize = FVector2D(SourceTexture->GetImportedSize());
		return FVector4(FVector2D::ZeroVector, SourceDimension / ImportedSize);
	}
	
	return FVector4(0, 0, 0, 0);
}

void USprite2D::NotifySpriteTextureChanged(bool bTextureChanged, bool bUVChanged)
{
	for (auto& Listener : Listeners)
	{
		if (Listener.IsValid())
		{
			ISpriteTextureAtlasListenerInterface* SpriteTextureAtlasListener = Cast<ISpriteTextureAtlasListenerInterface>(Listener.Get());
			if (SpriteTextureAtlasListener)
			{
				SpriteTextureAtlasListener->NotifySpriteTextureChanged(bTextureChanged, bUVChanged);
			}
		}
	}
}

/////////////////////////////////////////////////////
