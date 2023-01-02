#pragma once

#include "CoreMinimal.h"
#include "ComponentAssetBroker.h"
#include "Core/Layout/ChildWidgetActorComponent.h"

//////////////////////////////////////////////////////////////////////////
// FChildWidgetActorCompoentBroker

class FChildWidgetActorCompoentBroker : public IComponentAssetBroker
{
public:
	UClass* GetSupportedAssetClass() override
	{
		return UBlueprint::StaticClass();
	}

	virtual bool AssignAssetToComponent(UActorComponent* InComponent, UObject* InAsset) override
	{
		if (UChildWidgetActorComponent* ChildWidgetActorComp = Cast<UChildWidgetActorComponent>(InComponent))
		{
			UClass* Class = Cast<UClass>(InAsset);
			if (Class == nullptr)
			{
				if (UBlueprint* BP = Cast<UBlueprint>(InAsset))
				{
					Class = *(BP->GeneratedClass);
				}
			}
			if (Class && Class->IsChildOf<AActor>())
			{
				ChildWidgetActorComp->SetChildWidgetActorClass(Class);
				return true;
			}
		}

		return false;
	}

	virtual UObject* GetAssetFromComponent(UActorComponent* InComponent) override
	{
		if (UChildWidgetActorComponent* ChildWidgetActorComp = Cast<UChildWidgetActorComponent>(InComponent))
		{
			return UBlueprint::GetBlueprintFromClass(*(ChildWidgetActorComp->GetChildWidgetActorClass()));
		}
		return NULL;
	}
};
