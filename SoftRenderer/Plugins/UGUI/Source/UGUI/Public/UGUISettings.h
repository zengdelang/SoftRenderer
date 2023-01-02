#pragma once

#include "CoreMinimal.h"
#include "UIEnumModifier.h"
#include "Core/VertexModifiers/ShadowSubComponent.h"
#include "Core/Widgets/Text/TextEmojiSheet.h"
#include "UGUISettings.generated.h"

UCLASS(config = UGUI, defaultconfig)
class UGUI_API UUGUISettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category = MeshEffect, meta = (Enum = "ECustomExternalEffectType", Name = "You can have up to {0} custom external effect type for your project", SearchString = "Custom External Effect Type"))
	FUIEnumModifier ExternalEffectType;

	UPROPERTY(config, EditAnywhere, Category = MeshEffect)
	TMap<TEnumAsByte<ECustomExternalEffectType>, FCustomExternalEffect> CustomEffectMap;

	UPROPERTY(config, EditAnywhere, Category = Text)
	TMap<FName, FLinearColor> TextColorTags;

	UPROPERTY(config, EditAnywhere, Category = SafeZone)
	uint8 bSafeZoneHorizontalAlign : 1;

	UPROPERTY(config, EditAnywhere, Category = SafeZone)
	uint8 bSafeZoneVerticalAlign : 1;
	
public:
	UPROPERTY(config,EditAnywhere,Category = Emoji)
	uint8 bShowTextEmojiImageComponent : 1;

	UPROPERTY(config,EditAnywhere,Category = Emoji)
	uint8 bShowRichTextDebugRegion : 1;

	UPROPERTY(config)
	uint8 bVisibilityForEditor : 1;

	UPROPERTY(config, EditAnywhere, Category = UI)
	FLinearColor BlurDisabledColor;

	UPROPERTY(config, EditAnywhere, Category = UI)
	FVector2D UIRootSizeDelta;
	
public:
#if WITH_EDITORONLY_DATA
	DECLARE_EVENT(UUGUISettings, FOnCustomEffectChanged)
	FOnCustomEffectChanged OnCustomEffectChanged;
#endif
	
public:
	static UUGUISettings* Get() { return CastChecked<UUGUISettings>(UUGUISettings::StaticClass()->GetDefaultObject()); }

	virtual void PostInitProperties() override;

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
};
