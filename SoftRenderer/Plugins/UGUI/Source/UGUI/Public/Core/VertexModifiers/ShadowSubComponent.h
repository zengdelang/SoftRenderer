#pragma once

#include "CoreMinimal.h"
#include "BaseMeshEffectSubComponent.h"
#include "ShadowSubComponent.generated.h"

UENUM(BlueprintType)
enum ECustomExternalEffectType
{
	EffectType_None UMETA(DisplayName = "None"),
	EffectType_Type1 UMETA(Hidden),
	EffectType_Type2 UMETA(Hidden),
	EffectType_Type3 UMETA(Hidden),
	EffectType_Type4 UMETA(Hidden),
	EffectType_Type5 UMETA(Hidden),
	EffectType_Type6 UMETA(Hidden),
	EffectType_Type7 UMETA(Hidden),
	EffectType_Type8 UMETA(Hidden),
	EffectType_Type9 UMETA(Hidden),
	EffectType_Type10 UMETA(Hidden),
	EffectType_Type11 UMETA(Hidden),
	EffectType_Type12 UMETA(Hidden),
	EffectType_Type13 UMETA(Hidden),
	EffectType_Type14 UMETA(Hidden),
	EffectType_Type15 UMETA(Hidden),
	EffectType_Type16 UMETA(Hidden),
	EffectType_Type17 UMETA(Hidden),
	EffectType_Type18 UMETA(Hidden),
	EffectType_Type19 UMETA(Hidden),
	EffectType_Type20 UMETA(Hidden),
	EffectType_Type21 UMETA(Hidden),
	EffectType_Type22 UMETA(Hidden),
	EffectType_Type23 UMETA(Hidden),
	EffectType_Type24 UMETA(Hidden),
	EffectType_Type25 UMETA(Hidden),
	EffectType_Type26 UMETA(Hidden),
	EffectType_Type27 UMETA(Hidden),
	EffectType_Type28 UMETA(Hidden),
	EffectType_Type29 UMETA(Hidden),
	EffectType_Type30 UMETA(Hidden),
	EffectType_Type31 UMETA(Hidden),
	EffectType_Type32 UMETA(Hidden),
	EffectType_Type_Max UMETA(Hidden)
};

USTRUCT()
struct FCustomExternalEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = MeshEffect)
	FVector EffectDistance;

	UPROPERTY(EditAnywhere, Category = MeshEffect)
	FLinearColor EffectColor;
	
};

/**
 * Adds a shadow to a graphic using IVertexModifier.
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = Shadow))
class UGUI_API UShadowSubComponent : public UBaseMeshEffectSubComponent
{
	GENERATED_UCLASS_BODY()

protected:
	/**
	 * Color for the effect
	 */
	UPROPERTY(EditAnywhere, Category = Shadow)
	FLinearColor EffectColor;

	/**
	 * How far is the shadow from the graphic.
	 */
	UPROPERTY(EditAnywhere, Category = Shadow)
	FVector EffectDistance;

	/**
	 * Should the shadow inherit the alpha from the graphic?
	 */
	UPROPERTY(EditAnywhere, Category = Shadow)
	uint8 bUseGraphicAlpha : 1;

	UPROPERTY(EditAnywhere, Category = Shadow)
	uint8 bUseExternalEffect : 1;

	UPROPERTY(EditAnywhere, Category = Shadow)
	uint8 bFollowTransform : 1;

	UPROPERTY(EditAnywhere, Category = Shadow)
	FVector FixedLocalLocation;

	UPROPERTY(EditAnywhere, Category = Shadow)
	TEnumAsByte<ECustomExternalEffectType> ExternalEffectType;

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

public:
	UFUNCTION(BlueprintCallable, Category = Shadow)
	FLinearColor GetEffectColor() const
	{
		return EffectColor;
	}

	UFUNCTION(BlueprintCallable, Category = Shadow)
	void SetEffectColor(FLinearColor InEffectColor);

public:
	UFUNCTION(BlueprintCallable, Category = Shadow)
	FVector GetEffectDistance() const
	{
		return EffectDistance;
	}

	UFUNCTION(BlueprintCallable, Category = Shadow)
	void SetEffectDistance(FVector InEffectDistance);

public:
	UFUNCTION(BlueprintCallable, Category = Shadow)
	bool IsUseGraphicAlpha() const
	{
		return bUseGraphicAlpha;
	}
	
	UFUNCTION(BlueprintCallable, Category = Shadow)
	void SetUseGraphicAlpha(bool bInUseGraphicAlpha);

public:
	UFUNCTION(BlueprintCallable, Category = Shadow)
	bool IsUseExternalEffect() const
	{
		return bUseExternalEffect;
	}

	UFUNCTION(BlueprintCallable, Category = Shadow)
	void SetUseExternalEffect(bool bInUseExternalEffect);

public:
	UFUNCTION(BlueprintCallable, Category = Shadow)
	TEnumAsByte<ECustomExternalEffectType> GetExternalEffectType() const
	{
		return ExternalEffectType;
	}
	
	UFUNCTION(BlueprintCallable, Category = Shadow)
	void SetExternalEffectType(TEnumAsByte<ECustomExternalEffectType> InExternalEffectType);

public:
	UFUNCTION(BlueprintCallable, Category = Shadow)
	bool IsFollowTransform() const
	{
		return bFollowTransform;
	}

	UFUNCTION(BlueprintCallable, Category = Shadow)
	void SetFollowTransform(bool bInFollowTransform);

public:
	UFUNCTION(BlueprintCallable, Category = Shadow)
	FVector GetFixedLocalLocation() const
	{
		return FixedLocalLocation;
	}

	UFUNCTION(BlueprintCallable, Category = Shadow)
	void SetFixedLocalLocation(FVector InFixedLocalLocation);

protected:
#if WITH_EDITORONLY_DATA
	void OnCustomEffectChanged() const;
#endif
	
public:
	virtual void Awake() override;
	virtual void ModifyMesh(FVertexHelper& VertexHelper) override;

	virtual void OnTransformChanged() override;

protected:
	void ApplyShadow(TArray<FUIVertex>& Vertices, FLinearColor Color, int32 Start, int32 End, float X, float Y, float Z) const;

};
