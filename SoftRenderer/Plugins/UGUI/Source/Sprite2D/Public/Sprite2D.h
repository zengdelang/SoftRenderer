#pragma once

#include "CoreMinimal.h"
#include "Interfaces/SpriteTextureAtlasInterface.h"
#include "Sprite2D.generated.h"

UCLASS(BlueprintType)
class SPRITE2D_API USprite2D : public UObject, public ISpriteTextureAtlasInterface
{
	GENERATED_UCLASS_BODY()

	friend class USpriteMergeSubsystem;
	friend class FSpriteAtlasTexture;
	
protected:
	UPROPERTY(EditAnywhere, Category=Sprite)
	uint8 bUseDynamicAtlas : 1;

	uint8 bWaitingTextureData : 1;
	
	UPROPERTY(EditAnywhere, Category=Sprite)
	FVector2D SourceDimension;
	
	UPROPERTY(EditAnywhere, Category=Sprite)
	UTexture2D* SourceTexture;

	UPROPERTY(EditAnywhere, Category=Sprite)
	FName AtlasName;

	// The scaling factor between pixels and Unreal units (cm) (e.g., 0.64 would make a 64 pixel wide sprite take up 100 cm)
	UPROPERTY(EditAnywhere, Category=Sprite, meta = (DisplayName = "Pixels per unit"))
	float PixelsPerUnrealUnit;

protected:
	TWeakObjectPtr<UTexture2D> DynamicSpriteTexture;
	TWeakPtr<class FSpriteAtlasTexture> SpriteAtlasTexture;

	TSet<TWeakObjectPtr<UObject>> Listeners;

	FVector2D AtlasSourceUV;
	int32 ReferenceCount;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void FinishDestroy() override;
	
public:
	// ISpriteTextureAtlasInterface interface
	virtual void AddSpriteListener(UObject* Listener) override;
	virtual void RemoveSpriteListener(UObject* Listener) override;
	virtual void IncreaseReferenceCount() override;
	virtual void DecreaseReferenceCount() override;
	// End of ISpriteTextureAtlasInterface interface
	
public:
	UTexture2D* GetSpriteTexture();
	UTexture2D* GetSpriteTextureRaw() const;

#if WITH_EDITOR
	bool GetIsMerged();
	void RemergeSprite2D();
#endif

	FORCEINLINE int32 GetReferenceCount() const { return ReferenceCount; }
	
	FORCEINLINE UTexture2D* GetSourceTexture() const { return SourceTexture; }
	FORCEINLINE FVector2D GetSpriteSize() const { return SourceDimension; }
	
	// Return the scaling factor between pixels and Unreal units (cm)
	FORCEINLINE float GetPixelsPerUnrealUnit() const { return PixelsPerUnrealUnit; }

	FVector4 GetOuterUV();

#if WITH_EDITOR
	void SetImportSettings(bool bInUseDynamicAtlas, FVector2D InSourceDimension, UTexture2D* InSourceTexture,
		FName InAtlasName, float InPixelsPerUnrealUnit)
	{
		bUseDynamicAtlas = bInUseDynamicAtlas;
		SourceDimension = InSourceDimension;
		SourceTexture = InSourceTexture;
		AtlasName = InAtlasName;
		PixelsPerUnrealUnit = InPixelsPerUnrealUnit;
	}
	
	void SetSourceTextureForEditor(UTexture2D* InSourceTexture)
	{
		SourceTexture = InSourceTexture;
	}

	void GetSimpleRenderDataForThumbnail(TArray<FVector4>& RenderData, FBoxSphereBounds& Bounds) const
	{
		static FVector SpriteAxisX(1.0f, 0.0f, 0.0f);
		static FVector SpriteAxisY(0.0f, 0.0f, 1.0f);
		static FVector SpriteAxisZ(0.0f, 1.0f, 0.0f);
		
		RenderData.Empty();
		RenderData.Add(FVector4(SourceDimension.X * 0.5, SourceDimension.Y * 0.5, 1, 0));
		RenderData.Add(FVector4(-SourceDimension.X * 0.5, SourceDimension.Y * 0.5, 0, 0));
		RenderData.Add(FVector4(-SourceDimension.X * 0.5, -SourceDimension.Y * 0.5, 0, 1));
		RenderData.Add(FVector4(SourceDimension.X * 0.5, -SourceDimension.Y * 0.5, 1, 1));
        RenderData.Add(FVector4(SourceDimension.X * 0.5, SourceDimension.Y * 0.5, 1, 0));
        RenderData.Add(FVector4(-SourceDimension.X * 0.5, -SourceDimension.Y * 0.5, 0, 1));

		FBox BoundingBox(ForceInit);
	
		for (int32 VertexIndex = 0; VertexIndex < RenderData.Num(); ++VertexIndex)
		{
			const FVector4& VertXYUV = RenderData[VertexIndex];
			const FVector Vert((SpriteAxisX * VertXYUV.X) + (SpriteAxisY * VertXYUV.Y));
			BoundingBox += Vert;
		}
	
		// Make the whole thing a single unit 'deep'
		const FVector HalfThicknessVector = 0.5f * SpriteAxisZ;
		BoundingBox += -HalfThicknessVector;
		BoundingBox += HalfThicknessVector;

		Bounds = BoundingBox;
	}

	void SetSourceDimensionForEditor(FVector2D InSourceDimension)
	{
		SourceDimension = InSourceDimension;
	}
	
	DECLARE_MULTICAST_DELEGATE(FOnSpriteChange)
	FOnSpriteChange OnSpriteChange;
#endif

protected:
	void NotifySpriteTextureChanged(bool bTextureChanged, bool bUVChanged);
	
};
