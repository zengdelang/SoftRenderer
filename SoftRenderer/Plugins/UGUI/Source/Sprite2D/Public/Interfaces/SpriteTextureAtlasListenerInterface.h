#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SpriteTextureAtlasListenerInterface.generated.h"

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class SPRITE2D_API USpriteTextureAtlasListenerInterface : public UInterface
{
	GENERATED_BODY()
};

class SPRITE2D_API ISpriteTextureAtlasListenerInterface
{
	GENERATED_BODY()

public:
	virtual void NotifySpriteTextureChanged(bool bTextureChanged, bool bUVChanged) = 0;
	
};
