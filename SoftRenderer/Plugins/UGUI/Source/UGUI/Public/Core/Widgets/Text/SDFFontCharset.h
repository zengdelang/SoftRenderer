#pragma once

#include "CoreMinimal.h"
#include "SDFFontCharset.generated.h"

UENUM()
enum class ESDFFontStyle : uint8
{
	Normal,
	Bold,
	Italic,
	BoldAndItalic,
};

UCLASS(BlueprintType, EditInlineNew)
class UGUI_API USDFFontCharset : public UObject
{
	GENERATED_UCLASS_BODY()

#if WITH_EDITORONLY_DATA
public:
	UPROPERTY()
	FString Name = "New Charset";
	
	/**
	 * Character set, e.g. [0x00, 0x80], "abc"
	 */
	UPROPERTY(EditAnywhere, Category=FontCharset)
	FString Charset;

	UPROPERTY(EditAnywhere, Category=FontCharset)
	ESDFFontStyle FontStyle;

	UPROPERTY(EditAnywhere, Category=FontCharset)
	FString FontFilename;
	
	UPROPERTY(EditAnywhere, Category=FontCharset)
	int32 FaceIndex;

	UPROPERTY(EditAnywhere, Category=FontCharset, meta = (ClampMin = "1", ClampMax = "300", UIMin = "1", UIMax = "300"))
	int32 FontScale;

	UPROPERTY(EditAnywhere, Category=FontCharset, meta = (ClampMin = "1", ClampMax = "100", UIMin = "1", UIMax = "100"))
	int32 PxRange;

	UPROPERTY(EditAnywhere, Category="SDF Generator")
	uint8 bEnabled : 1;
#endif
	
};
