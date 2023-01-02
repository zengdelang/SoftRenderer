#include "UGUISettings.h"

/////////////////////////////////////////////////////
// UUGUISettings

UUGUISettings::UUGUISettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bSafeZoneHorizontalAlign = false;
	bSafeZoneVerticalAlign = false;
	bShowTextEmojiImageComponent = false;
	bShowRichTextDebugRegion = false;
	bVisibilityForEditor = true;

	BlurDisabledColor = FLinearColor(0, 0, 0, 0.5);

	UIRootSizeDelta = FVector2D(1920, 1080);
	
	TextColorTags.Emplace(TEXT("Blue"), FColor::FromHex("#0000FF").ReinterpretAsLinear());
	TextColorTags.Emplace(TEXT("Black"), FColor::FromHex("#000000").ReinterpretAsLinear());
	TextColorTags.Emplace(TEXT("Brown"), FColor::FromHex("#A52A2A").ReinterpretAsLinear());
	TextColorTags.Emplace(TEXT("Green"), FColor::FromHex("#008000").ReinterpretAsLinear());
	TextColorTags.Emplace(TEXT("Orange"), FColor::FromHex("#FFA500").ReinterpretAsLinear());
	TextColorTags.Emplace(TEXT("Red"), FColor::FromHex("#FF0000").ReinterpretAsLinear());
	TextColorTags.Emplace(TEXT("White"), FColor::FromHex("#FFFFFF").ReinterpretAsLinear());
	TextColorTags.Emplace(TEXT("Purple"), FColor::FromHex("#800080").ReinterpretAsLinear());
	TextColorTags.Emplace(TEXT("Yellow"), FColor::FromHex("#FFFF00").ReinterpretAsLinear());
}

void UUGUISettings::PostInitProperties()
{
	Super::PostInitProperties();
#if WITH_EDITORONLY_DATA
	ExternalEffectType.SetEnumInfo(StaticEnum<ECustomExternalEffectType>());
#endif
}

#if WITH_EDITORONLY_DATA

void UUGUISettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UUGUISettings, bVisibilityForEditor) || MemberPropertyName == NAME_None)
	{
		if (!FUnrealEdMisc::Get().IsDeletePreferences())
		{
			SaveConfig();
		}
	}
}

void UUGUISettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);	
	
	OnCustomEffectChanged.Broadcast();
}

#endif

/////////////////////////////////////////////////////
