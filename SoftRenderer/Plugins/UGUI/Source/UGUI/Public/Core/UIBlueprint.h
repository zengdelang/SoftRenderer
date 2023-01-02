#pragma once

#include "Engine/Blueprint.h"
#include "UIBlueprint.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBlueprintPreSave, UBlueprint*);

UCLASS(Blueprintable, BlueprintType)
class UGUI_API UUIBlueprint : public UBlueprint
{
    GENERATED_UCLASS_BODY()

#if WITH_EDITORONLY_DATA
public:
    static FOnBlueprintPreSave OnBlueprintPreSave;
    
    virtual void PreSave(const ITargetPlatform* TargetPlatform) override;
#endif
    
};
