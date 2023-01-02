#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "SDFFontFactory.generated.h"

UCLASS()
class USDFFontFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

public:
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

};
