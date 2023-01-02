#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SpriteTextureAtlasInterface.generated.h"

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class SPRITE2D_API USpriteTextureAtlasInterface : public UInterface
{
	GENERATED_BODY()
};

class SPRITE2D_API ISpriteTextureAtlasInterface
{
	GENERATED_BODY()

public:
	virtual void AddSpriteListener(UObject* Listener) = 0;
	virtual void RemoveSpriteListener(UObject* Listener) = 0;
	virtual void IncreaseReferenceCount() = 0;
	virtual void DecreaseReferenceCount() = 0;
	
};
