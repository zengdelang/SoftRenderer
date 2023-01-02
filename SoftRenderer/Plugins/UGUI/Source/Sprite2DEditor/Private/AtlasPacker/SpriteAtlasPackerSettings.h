#pragma once

#include "CoreMinimal.h"
#include "SpriteAtlasPackerSettings.generated.h"
 
UCLASS(BlueprintType)
class USpriteAtlasPackerSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, Category=Atlas)
	uint8 bUseDynamicAtlas : 1;

	UPROPERTY(EditAnywhere, Category=Atlas)
	FName AtlasName;

	UPROPERTY(EditAnywhere, Category=Atlas)
	FString ImportPath;

	UPROPERTY(EditAnywhere, Category=Atlas, meta = (DisplayName = "Pixels per unit"))
	float PixelsPerUnrealUnit;
	
};

