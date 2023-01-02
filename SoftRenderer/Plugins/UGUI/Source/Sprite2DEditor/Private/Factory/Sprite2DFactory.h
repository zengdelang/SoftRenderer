#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Sprite2DFactory.generated.h"

UCLASS()
class USprite2DFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

public:
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	class UTexture2D* InitialTexture;
};
